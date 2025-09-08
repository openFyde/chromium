// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fydeos/misc/fydeos_email_subscription.h"

#include "chrome/browser/browser_process.h"
#include "services/network/public/cpp/resource_request.h"
#include "chrome/browser/net/system_network_context_manager.h"
#include "services/network/public/cpp/simple_url_loader.h"
#include "net/http/http_status_code.h"
#include "services/network/public/mojom/url_response_head.mojom.h"
#include "url/url_util.h"

#include "base/strings/utf_string_conversions.h"
#include "base/strings/stringprintf.h"

#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ash/profiles/profile_helper.h"
#include "components/user_manager/user.h"
#include "base/system/sys_info.h"
#include "base/json/json_writer.h"

#include "fydeos/build/config/buildflags.h"

namespace fydeos {

namespace {
  const char kJSONContentType[] = "application/json";
#if BUILDFLAG(USE_FYDEOS_COM)
  const char kFydeOSSubscriptionUrl[] =
    "https://apis.fydeos.com/mailing/oobe/completion";
#else
  const char kFydeOSSubscriptionUrl[] =
    "https://apis.fydeos.io/mailing/oobe/completion";
#endif
  const size_t kMaxMessageSize = 1024 * 1;  // 1MB

  void OnSimpleLoaderComplete(
      std::unique_ptr<network::SimpleURLLoader> url_loader,
      std::unique_ptr<std::string> response_body) {
    int response_code = 0;
    if (url_loader->ResponseInfo() && url_loader->ResponseInfo()->headers) {
      response_code = url_loader->ResponseInfo()->headers->response_code();
    }
    VLOG(2) << "subscription service response code: " << response_code;
    if (response_body) {
      VLOG(2) << "received from subscription service:" << *response_body;
    }
    if (response_code != net::HTTP_OK) {
      VLOG(2) << "subscribtion service net error:" << url_loader->NetError();
    }
  }

  std::string GenerateRequestBodyWithSysInfo(const std::string& name, const std::string& email,
                                              bool email_opt_in, bool improve_plan_opt_in) {
    const std::string version = base::SysInfo::GetLsbFydeReleaseVersion();
    const std::string board_name = base::SysInfo::GetLsbReleaseBoard();

    base::Value::Dict post_body_value;
    post_body_value.Set("name", name);
    post_body_value.Set("email", email);
    post_body_value.Set("os_version", version);
    post_body_value.Set("board_name", board_name);
    post_body_value.Set("subscribe_newsletter", email_opt_in);
    post_body_value.Set("join_improvement_plan", improve_plan_opt_in);
    std::string post_body;
    bool write_success = base::JSONWriter::Write(post_body_value, &post_body);
    if (!write_success) {
      return std::string();
    }

    return post_body;
  }

  void StartPost(const std::string& name, const std::string& email,
                 bool email_opt_in, bool improve_plan_opt_in) {
    if (!g_browser_process->system_network_context_manager()->HasInstance()) {
      return;
    }
    network::mojom::URLLoaderFactory* loader_factory =
      g_browser_process->system_network_context_manager()
                         ->GetURLLoaderFactory();
    if (!loader_factory) {
      return;
    }

    net::NetworkTrafficAnnotationTag traffic_annotation =
      net::DefineNetworkTrafficAnnotation("fydeos_email_subscription", R"(
        semantics {
          sender: "FydeOS Email Subscription"
          description:
            "Subscribe to fydeos news"
          trigger:
            "Choose opt-in during OOBE"
          data:
            "1- name.\n"
            "2- email."
          destination: FYDEOS_SUBSCRIPTION_SERVICE
        }
        policy {
          cookies_allowed: NO
          policy_exception_justification: "Not implemented."
        })");

    auto resource_request = std::make_unique<network::ResourceRequest>();
    resource_request->url = GURL(kFydeOSSubscriptionUrl);
    resource_request->credentials_mode = network::mojom::CredentialsMode::kOmit;
    resource_request->method = "POST";
    std::unique_ptr<network::SimpleURLLoader> simple_loader =
      network::SimpleURLLoader::Create(std::move(resource_request),
          traffic_annotation);
    const std::string body = GenerateRequestBodyWithSysInfo(name, email, email_opt_in, improve_plan_opt_in);
    VLOG(4) << "subscription request, post data: " << body;
    if (body.empty()) {
      return;
    }
    simple_loader->AttachStringForUpload(body, kJSONContentType);
    simple_loader->SetTimeoutDuration(base::Minutes(1));
    int retry_mode = network::SimpleURLLoader::RETRY_ON_NETWORK_CHANGE
                      | network::SimpleURLLoader::RETRY_ON_NAME_NOT_RESOLVED;
    simple_loader->SetRetryOptions(3, retry_mode);

    network::SimpleURLLoader* simple_loader_ptr = simple_loader.get();
    simple_loader_ptr->DownloadToString(
        loader_factory,
        base::BindOnce(&OnSimpleLoaderComplete, std::move(simple_loader)),
        kMaxMessageSize);
  }
}  // namespace

namespace misc {

  void Subscribe(Profile* profile, bool email_opt_in, bool improve_plan_opt_in) {
    if (!profile) {
      return;
    }
    user_manager::User* user =
      ::ash::ProfileHelper::Get()->GetUserByProfile(profile);
    if (!user) {
      return;
    }
    const std::string email = user->GetAccountId().GetUserEmail();
    if (email.empty()) {
      return;
    }
    std::u16string name16 = user->GetGivenName();
    if (name16.empty()) name16 = user->GetDisplayName();
    const std::string name = base::UTF16ToUTF8(name16);

    StartPost(name, email, email_opt_in, improve_plan_opt_in);
  }

}  // namespace misc
}  // namespace fydeos
