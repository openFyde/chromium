// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/extensions/external_component_loader.h"

#include "base/values.h"
#include "build/branding_buildflags.h"
#include "build/build_config.h"
#include "build/chromeos_buildflags.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/extensions/component_extensions_allowlist/allowlist.h"
#include "chrome/common/buildflags.h"
#include "chrome/common/extensions/extension_constants.h"
#include "extensions/common/constants.h"
#include "extensions/common/extension_urls.h"
#include "extensions/common/feature_switch.h"
#include "extensions/common/manifest.h"

#if BUILDFLAG(IS_CHROMEOS)
#include "chrome/browser/policy/profile_policy_connector.h"
#endif

#include "base/strings/stringprintf.h"
#include "fydeos/switches/services/services_switches.h"

namespace {

void AddFydeOSExtensions(base::Value::Dict& prefs) {
  const char update_url_template[] =
    "%s/update/%s/updates.xml";
  std::vector<std::string> preinstalled_apps = {
    "hidnajblbifdkmheebalalchohohmaef",  // system controller
    "iakadpgajjigiaojnbdmodlngmbkfhag",  // start arc settings
    "mofiofjpikncjaigmdlblhojbnkabako",  // store
    // "fogdcaodknbhigpklbhepedofamkfbln", // rdp
    "nfglebjgiflmmcdddkbcbgmdkomlfcpa",  // rime
  };
  const std::string base_update_url =
    fydeos::switches::GetFydeOSWebStoreUpdateUrl();
  std::string update_url;
  for (const auto& app_id : preinstalled_apps) {
    base::SStringPrintf(&update_url, update_url_template,
        base_update_url.c_str(), app_id.c_str());
    prefs.SetByDottedPath(app_id + ".external_update_url", update_url);
  }
}

}  // namespace

namespace extensions {

ExternalComponentLoader::ExternalComponentLoader(Profile* profile)
    : profile_(profile) {}

ExternalComponentLoader::~ExternalComponentLoader() {}

void ExternalComponentLoader::StartLoading() {
  auto prefs = base::Value::Dict();
#if BUILDFLAG(GOOGLE_CHROME_BRANDING)
  AddExternalExtension(extension_misc::kInAppPaymentsSupportAppId, prefs);
#endif  // BUILDFLAG(GOOGLE_CHROME_BRANDING)

#if BUILDFLAG(IS_CHROMEOS)
  {
    // Only load the Assessment Assistant if the current session is managed.
    if (profile_->GetProfilePolicyConnector()->IsManaged()) {
      AddExternalExtension(extension_misc::kAssessmentAssistantExtensionId,
                           prefs);
    }
  }
#endif

  AddFydeOSExtensions(prefs);

  LoadFinished(std::move(prefs));
}

void ExternalComponentLoader::AddExternalExtension(
    const std::string& extension_id,
    base::Value::Dict& prefs) {
  if (!IsComponentExtensionAllowlisted(extension_id))
    return;

  prefs.SetByDottedPath(extension_id + ".external_update_url",
                        extension_urls::GetWebstoreUpdateUrl().spec());
}

}  // namespace extensions
