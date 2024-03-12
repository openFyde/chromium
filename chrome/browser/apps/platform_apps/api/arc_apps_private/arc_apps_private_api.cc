// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/apps/platform_apps/api/arc_apps_private/arc_apps_private_api.h"

#include <string>
#include <vector>

#include "base/check_op.h"
#include "base/logging.h"
#include "base/no_destructor.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "chrome/browser/ash/app_list/arc/arc_app_utils.h"
#include "chrome/browser/ash/login/demo_mode/demo_session.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/common/apps/platform_apps/api/arc_apps_private.h"
#include "ui/events/event_constants.h"
#include "chrome/browser/ash/arc/intent_helper/arc_intent_helper_mojo_ash.h"
#include "chrome/browser/ash/crosapi/arc_ash.h"
#include "ash/components/arc/session/arc_service_manager.h"
#include "components/arc/intent_helper/arc_intent_helper_bridge.h"
#include "ash/components/arc/session/arc_bridge_service.h"
#include "chrome/browser/ash/file_manager/path_util.h"
#include "base/json/json_writer.h"
#include "base/values.h"
#include "chrome/browser/apps/app_service/file_utils.h"
#include "chrome/browser/ash/arc/arc_util.h"

class GURL;
using namespace arc;
using namespace file_manager::util;

namespace {

  constexpr char kInstallAction[] = "io.fydeos.arc.ArcGateway.action.INSTALL_PACKAGE";

  constexpr char kInstallPackageName[] = "io.fydeos.arc.ArcGateway";
  constexpr char kTypeName[] = "application/vnd.android.package-archive";
  constexpr char KInstallActivityName[] = "io.fydeos.arc.ArcGateway.InstallationActivity";
//  constexpr char kInstallServiceClassName[] = "io.fydeos.arc.ArcGateway.InstallationService";
  constexpr char kInstallExtraVerifyUrl[] = "io.fydeos.arc.ArcGateway.extra.VERIFY_URL";

  ArcIntentHelperMojoDelegate::IntentInfo CreateIntentInfo(const GURL& arc_url, const std::string& verify_url) {
    base::flat_map<std::string, std::string> extra = {
      {kInstallExtraVerifyUrl, verify_url},
      {"arc_share_url", arc_url.possibly_invalid_spec()}};
    return ArcIntentHelperMojoDelegate::IntentInfo(
      kInstallAction, /*categories=*/absl::nullopt, arc_url.possibly_invalid_spec(),
      kTypeName, /*ui_bypassed*/ true, extra
    );
  }

  bool HandleIntent(ArcIntentHelperMojoDelegate::IntentInfo& intent) {
//  bool HandleIntent(const std::string& arc_url, const std::string& verify_url) {
		auto* arc_service_manager = arc::ArcServiceManager::Get();
	  if (!arc_service_manager) {
	    LOG(WARNING) << "ARC is not ready";
	    return false;
	  }

	  auto* intent_helper_holder =
	      arc_service_manager->arc_bridge_service()->intent_helper();
	  if (!intent_helper_holder->IsConnected()) {
	    LOG(WARNING) << "ARC intent helper instance is not ready.";
	    return false;
	  }
		auto* instance =
      ARC_GET_INSTANCE_FOR_METHOD(intent_helper_holder, HandleIntent);
	  if (!instance) {
	    LOG(WARNING) << "HandleIntent is not supported.";
	    return false;
	  }

		arc::mojom::IntentInfoPtr converted_intent = arc::mojom::IntentInfo::New();
	  converted_intent->action = intent.action;
	  converted_intent->categories = intent.categories;
	  converted_intent->data = intent.data;
	  converted_intent->type = intent.type;
	  converted_intent->ui_bypassed = intent.ui_bypassed;
	  converted_intent->extras = intent.extras;
	  instance->HandleIntent(std::move(converted_intent),
                         arc::mojom::ActivityName::New(
                             kInstallPackageName, KInstallActivityName));
   /*
    base::Value::Dict value;
    std::string extras;
    value.Set("data", arc_url);
    value.Set(kInstallExtraVerifyUrl, verify_url);
    base::JSONWriter::Write(value, &extras);
    instance->SendBroadcast(kInstallAction, kInstallPackageName, std::string(), extras);
    */
    return true;
  }

  void OnUrlResloved(const std::string& verify_url, const std::vector<GURL>& content_urls) {
    if (content_urls.size() < 1 || content_urls[0].is_empty()) {
      LOG(WARNING) << __func__ << " content_urls is empty";
      return;
    }
    LOG(INFO) << __func__ << " arc_url:" << content_urls[0];
    ArcIntentHelperMojoDelegate::IntentInfo intent = CreateIntentInfo(content_urls[0], verify_url);
    HandleIntent(intent);
  }

  bool ConvertArcUrlAndSendToArc(Profile *profile, const base::FilePath& file_path, const std::string& verify_url) {
    base::FilePath abs_path;
    base::FilePath primary_downloads = GetDownloadsFolderForProfile(profile);
    abs_path = primary_downloads.Append(file_path);
    storage::FileSystemURL file_url =  apps::GetFileSystemURL(profile, apps::GetFileSystemUrl(profile, abs_path));
    if (!file_url.is_valid()) {
      LOG(ERROR) << "failed to get system url, abs_path:" << abs_path << " file system url:" << file_url.DebugString();
      return false;
    }
    LOG(INFO) << "abs path:" << abs_path <<  " cracked file system url:" << file_url.DebugString();
    std::vector<storage::FileSystemURL> file_system_urls {file_url};
    ConvertToContentUrlsAndShare(profile, file_system_urls,
      base::BindOnce(&OnUrlResloved, std::move(verify_url)));
    return true;
  }

}// no name

namespace chrome_apps {
namespace api {
// static
extensions::BrowserContextKeyedAPIFactory<ArcAppsPrivateAPI>*
ArcAppsPrivateAPI::GetFactoryInstance() {
  static base::NoDestructor<
      extensions::BrowserContextKeyedAPIFactory<ArcAppsPrivateAPI>>
      instance;
  return instance.get();
}

ArcAppsPrivateAPI::ArcAppsPrivateAPI(content::BrowserContext* context)
    : context_(context) {
  extensions::EventRouter::Get(context_)->RegisterObserver(
      this, api::arc_apps_private::OnInstalled::kEventName);
  extensions::EventRouter::Get(context_)->RegisterObserver(
      this, api::arc_apps_private::OnRemoved::kEventName);
  extensions::EventRouter::Get(context_)->RegisterObserver(
      this, api::arc_apps_private::OnInstallationStarted::kEventName);
  extensions::EventRouter::Get(context_)->RegisterObserver(
      this, api::arc_apps_private::OnInstallationProgressChanged::kEventName);
  extensions::EventRouter::Get(context_)->RegisterObserver(
      this, api::arc_apps_private::OnInstallationFinished::kEventName);
}

ArcAppsPrivateAPI::~ArcAppsPrivateAPI() = default;

void ArcAppsPrivateAPI::Shutdown() {
  extensions::EventRouter::Get(context_)->UnregisterObserver(this);
  scoped_prefs_observation_.Reset();
}

void ArcAppsPrivateAPI::OnListenerAdded(
    const extensions::EventListenerInfo& details) {
  auto* prefs = ArcAppListPrefs::Get(Profile::FromBrowserContext(context_));
  if (prefs && !scoped_prefs_observation_.IsObservingSource(prefs))
    scoped_prefs_observation_.Observe(prefs);
}

void ArcAppsPrivateAPI::OnListenerRemoved(
    const extensions::EventListenerInfo& details) {
  extensions::EventRouter *router = extensions::EventRouter::Get(context_);
  if (!router->HasEventListener(api::arc_apps_private::OnInstalled::kEventName) &&
      !router->HasEventListener(api::arc_apps_private::OnRemoved::kEventName) &&
      !router->HasEventListener(api::arc_apps_private::OnInstallationStarted::kEventName) &&
      !router->HasEventListener(api::arc_apps_private::OnInstallationProgressChanged::kEventName) &&
      !router->HasEventListener(api::arc_apps_private::OnInstallationFinished::kEventName)) {
    scoped_prefs_observation_.Reset();
  }
}

void ArcAppsPrivateAPI::OnAppRegistered(
    const std::string& app_id,
    const ArcAppListPrefs::AppInfo& app_info) {
  if (!app_info.launchable)
    return;
  api::arc_apps_private::AppInfo app_info_result;
  app_info_result.package_name = app_info.package_name;
  auto event = std::make_unique<extensions::Event>(
      extensions::events::ARC_APPS_PRIVATE_ON_INSTALLED,
      api::arc_apps_private::OnInstalled::kEventName,
      api::arc_apps_private::OnInstalled::Create(app_info_result), context_);
  extensions::EventRouter::Get(context_)->BroadcastEvent(std::move(event));
}

void ArcAppsPrivateAPI::OnPackageRemoved(const std::string& package_name,
                                         bool uninstalled) {

  api::arc_apps_private::AppInfo app_info_result;
  app_info_result.package_name = package_name;
  auto event = std::make_unique<extensions::Event>(
      extensions::events::ARC_APPS_PRIVATE_ON_REMOVED,
      api::arc_apps_private::OnRemoved::kEventName,
      api::arc_apps_private::OnRemoved::Create(app_info_result), context_);
  extensions::EventRouter::Get(context_)->BroadcastEvent(std::move(event));
}

void ArcAppsPrivateAPI::OnInstallationStarted(const std::string& package_name) {
  api::arc_apps_private::AppInfo app_info_result;
  app_info_result.package_name = package_name;
  LOG(INFO) << __func__ << package_name;
  auto event = std::make_unique<extensions::Event>(
      extensions::events::ARC_APPS_PRIVATE_ON_INSTALLATION_STARTED,
      api::arc_apps_private::OnInstallationStarted::kEventName,
      api::arc_apps_private::OnInstallationStarted::Create(app_info_result), context_);
  extensions::EventRouter::Get(context_)->BroadcastEvent(std::move(event));
}

void ArcAppsPrivateAPI::OnInstallationProgressChanged(const std::string& package_name,
                                                      float progress) {
  api::arc_apps_private::AppInfo app_info_result;
  app_info_result.package_name = package_name;
  LOG(INFO) << __func__ << package_name;
  auto event = std::make_unique<extensions::Event>(
      extensions::events::ARC_APPS_PRIVATE_ON_INSTALLATION_PROGRESS_CHANGED,
      api::arc_apps_private::OnInstallationProgressChanged::kEventName,
      api::arc_apps_private::OnInstallationProgressChanged::Create(app_info_result, progress), context_);
  extensions::EventRouter::Get(context_)->BroadcastEvent(std::move(event));
}

void ArcAppsPrivateAPI::OnInstallationFinished(const std::string& package_name,
                                               bool success) {
  api::arc_apps_private::AppInfo app_info_result;
  app_info_result.package_name = package_name;
  LOG(INFO) << __func__ << package_name;
  auto event = std::make_unique<extensions::Event>(
      extensions::events::ARC_APPS_PRIVATE_ON_INSTALLATION_FINISHED,
      api::arc_apps_private::OnInstallationFinished::kEventName,
      api::arc_apps_private::OnInstallationFinished::Create(app_info_result, success), context_);
  extensions::EventRouter::Get(context_)->BroadcastEvent(std::move(event));
}

ArcAppsPrivateGetLaunchableAppsFunction::
    ArcAppsPrivateGetLaunchableAppsFunction() = default;

ArcAppsPrivateGetLaunchableAppsFunction::
    ~ArcAppsPrivateGetLaunchableAppsFunction() = default;

ExtensionFunction::ResponseAction
ArcAppsPrivateGetLaunchableAppsFunction::Run() {
  auto* prefs =
      ArcAppListPrefs::Get(Profile::FromBrowserContext(browser_context()));
  if (!prefs)
    return RespondNow(Error("Not available"));

  std::vector<api::arc_apps_private::AppInfo> result;
  const std::vector<std::string> app_ids = prefs->GetAppIds();
  for (const auto& app_id : app_ids) {
    std::unique_ptr<ArcAppListPrefs::AppInfo> app_info = prefs->GetApp(app_id);
    if (app_info && app_info->launchable) {
      api::arc_apps_private::AppInfo app_info_result;
      app_info_result.package_name = app_info->package_name;
      if (app_info->version_name)
        app_info_result.version = app_info->version_name;
      result.push_back(std::move(app_info_result));
    }
  }
  return RespondNow(ArgumentList(
      api::arc_apps_private::GetLaunchableApps::Results::Create(result)));
}

ArcAppsPrivateLaunchAppFunction::ArcAppsPrivateLaunchAppFunction() = default;

ArcAppsPrivateLaunchAppFunction::~ArcAppsPrivateLaunchAppFunction() = default;

ExtensionFunction::ResponseAction ArcAppsPrivateLaunchAppFunction::Run() {
  absl::optional<api::arc_apps_private::LaunchApp::Params> params(
      api::arc_apps_private::LaunchApp::Params::Create(args()));
  EXTENSION_FUNCTION_VALIDATE(params.has_value());
  ArcAppListPrefs* prefs =
      ArcAppListPrefs::Get(Profile::FromBrowserContext(browser_context()));
  if (!prefs)
    return RespondNow(Error("Not available"));

  const std::string app_id = prefs->GetAppIdByPackageName(params->package_name);
  if (app_id.empty())
    return RespondNow(Error("App not found"));

  if (!arc::LaunchApp(
          browser_context(), app_id, ui::EF_NONE,
          arc::UserInteractionType::APP_STARTED_FROM_EXTENSION_API)) {
    return RespondNow(Error("Launch failed"));
  }

  ash::DemoSession::RecordAppLaunchSourceIfInDemoMode(
      ash::DemoSession::AppLaunchSource::kExtensionApi);

  return RespondNow(NoArguments());
}

ArcAppsPrivateRemovePackageFunction::ArcAppsPrivateRemovePackageFunction() = default;
ArcAppsPrivateRemovePackageFunction::~ArcAppsPrivateRemovePackageFunction() = default;

ExtensionFunction::ResponseAction ArcAppsPrivateRemovePackageFunction::Run() {
  absl::optional<api::arc_apps_private::RemovePackage::Params> params(
      api::arc_apps_private::RemovePackage::Params::Create(args()));
  EXTENSION_FUNCTION_VALIDATE(params.has_value());
  if (params->package_name.empty())
    return RespondNow(Error("Need package name"));
  ArcAppListPrefs* prefs =
      ArcAppListPrefs::Get(Profile::FromBrowserContext(browser_context()));
  if (!prefs)
    return RespondNow(Error("Not available"));
  const std::string app_id = prefs->GetAppIdByPackageName(params->package_name);
  if (app_id.empty())
    return RespondNow(Error("App not found"));
  arc::UninstallPackage(params->package_name);
  return RespondNow(NoArguments());
}

ArcAppsPrivateInstallPackageFromPathFunction::ArcAppsPrivateInstallPackageFromPathFunction() = default;
ArcAppsPrivateInstallPackageFromPathFunction::~ArcAppsPrivateInstallPackageFromPathFunction() = default;

ExtensionFunction::ResponseAction ArcAppsPrivateInstallPackageFromPathFunction::Run() {
  absl::optional<api::arc_apps_private::InstallPackageFromPath::Params> params(
      api::arc_apps_private::InstallPackageFromPath::Params::Create(args()));
  EXTENSION_FUNCTION_VALIDATE(params.has_value());
  if (params->relative_path.empty())
    return RespondNow(Error("Need apk download path"));
  LOG(INFO) << "path:" << params->relative_path;
  if (params->verify_url.empty())
    return RespondNow(Error("Need verify url"));
  base::FilePath apk_path = base::FilePath(params->relative_path);
  LOG(INFO) << "get relative_path:" << apk_path;
  if (!ConvertArcUrlAndSendToArc(Profile::FromBrowserContext(browser_context()), apk_path, params->verify_url))
		return RespondNow(Error("Failed to convert " + apk_path.value()));
/*
  LOG(INFO) << "arc url:" << arc_url;
  ArcIntentHelperMojoDelegate::IntentInfo intentInfo = CreateIntentInfo(arc_url, params->verify_url);
#if 0
	if (!HandleIntent(arc_url.spec(), params->relative_path))
#else
  if (!HandleIntent(intentInfo))
#endif
		return RespondNow(Error("Failed to send to arc container"));
*/
	return RespondNow(NoArguments());
}

}  // namespace api
}  // namespace chrome_apps
