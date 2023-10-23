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

#include "fydeos/build/config/buildflags.h"

namespace fydeos {

namespace {
  const char kFormEncodedContentType[] = "application/x-www-form-urlencoded";
#if BUILDFLAG(USE_FYDEOS_COM)
  const char kFydeOSSubscriptionUrl[] =
    "https://fydeos.com/content/wp-admin/admin-ajax.php";
#else
  const char kFydeOSSubscriptionUrl[] =
    "https://fydeos.io/content/wp-admin/admin-ajax.php";
#endif
  const size_t kMaxMessageSize = 1024 * 1;  // 1MB

  const char kFydeOSSubscriptionParamAction[] = "fluentform_submit";
  const char kFydeOSSubscriptionParamFormId[] = "5";

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

  const std::string EncodeQueryStringData(const std::string& str) {
    url::RawCanonOutputT<char> encoded;
    url::EncodeURIComponent(str.c_str(), str.length(), &encoded);
    std::string encoded_str = std::string(encoded.data(), encoded.length());
    return encoded_str;
  }

  std::string QueryStringifyParamsWithSysInfo(const std::string& name, const std::string& email,
                                              bool email_opt_in, bool improve_plan_opt_in) {
    const std::string encoded_name = EncodeQueryStringData(name);
    const std::string encoded_email = EncodeQueryStringData(email);
    const std::string version = base::SysInfo::GetLsbFydeReleaseVersion();
    const std::string board_name = base::SysInfo::GetLsbReleaseBoard();

    std::string data = base::StringPrintf("name=%s&email=%s&board=%s&osver=%s",
        encoded_name.c_str(), encoded_email.c_str(),
        board_name.c_str(), version.c_str());

    if (email_opt_in) {
      data += "&useroptions[]=newsletter";
    }
    if (improve_plan_opt_in) {
      data += "&useroptions[]=improvementplan";
    }

    url::RawCanonOutputT<char> percent_encoded_data;
    url::EncodeURIComponent(data.c_str(), data.length(), &percent_encoded_data);

    return std::string(percent_encoded_data.data(), percent_encoded_data.length());
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
    const std::string body = base::StringPrintf("data=%s&action=%s&form_id=%s",
        QueryStringifyParamsWithSysInfo(name, email, email_opt_in, improve_plan_opt_in).c_str(),
        kFydeOSSubscriptionParamAction,
        kFydeOSSubscriptionParamFormId);
    VLOG(4) << "subscription request, post data: " << body;
    simple_loader->AttachStringForUpload(body, kFormEncodedContentType);
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
