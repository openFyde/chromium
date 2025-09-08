// Copyright 2020 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fydeos/license/fydeos_license_fetcher.h"

#include <string>
#include "url/gurl.h"
#include "net/traffic_annotation/network_traffic_annotation.h"
#include "chrome/browser/browser_process.h"
/*
#include "net/url_request/url_fetcher.h"
#include "net/proxy_resolution/proxy_config_service_fixed.h"
#include "net/url_request/url_request_context_builder.h"
*/

#include "chrome/browser/policy/dm_token_utils.h"
#include "fydeos/switches/license/license_constants.h"

#include "services/network/public/cpp/resource_request.h"
#include "chrome/browser/net/system_network_context_manager.h"
#include "net/http/http_status_code.h"
#include "services/network/public/mojom/url_response_head.mojom.h"
#include "fydeos/switches/license/license_switches.h"

#include "net/base/url_util.h"
#include "base/system/sys_info.h"
#include "fydeos/license/fydeos_license_user_util.h"

namespace fydeos::license {
namespace {
  const char kFydeOSLicenseQueryPath[] = "/chromeos/";
  const int kFetcherTimeout = 1;
  static const size_t kMaxDownloadSize = 30 * 1024;

  const net::NetworkTrafficAnnotationTag kTrafficAnnotation =
    net::DefineNetworkTrafficAnnotation("fydeos_get_online_license", R"(
        semantics {
          sender: "FydeOS - FydeOS license API"
          description:
            "This request is used to fetch fydeos license "
            "authentication cookies."
          trigger:
            "This request is part of FydeOS online API, and is triggered if "
            "local license is failed. "
            "cookies is not required."
          data: "None."
          destination: FYDEOS_LICENSE_SERVICE
        }
        policy {
          cookies_allowed: NO
          cookies_store: "user"
          setting:
            "This feature cannot be disabled in settings, but if the user "
            "signs out of Chrome, this request would not be made."
          chrome_policy {
            SigninAllowed {
              policy_options {mode: MANDATORY}
              SigninAllowed: false
            }
          }
        })");

  GURL GetFullURL(const std::string& id,
                  const std::string& serial_number,
                  const bool is_new_license,
                  const std::string& oem_token,
                  const std::string& basic_token) {
    GURL url(
        fydeos::switches::GetFydeOSLicenseApiUrl() + kFydeOSLicenseQueryPath + id);
    url = net::AppendQueryParameter(url, "is_new", is_new_license ? "true" : "false");
    url = net::AppendQueryParameter(url, "serialNumber", serial_number);
    url = net::AppendQueryParameter(
        url, "board", base::SysInfo::GetLsbReleaseBoard());

    policy::DMToken dmToken = policy::GetDeviceDMToken();
    if (dmToken.is_valid()) {
      url = net::AppendQueryParameter(url, "dmToken", dmToken.value());
    }
    if (!oem_token.empty()) {
      url = net::AppendQueryParameter(url, "oemToken", oem_token);
    }
    if (!basic_token.empty()) {
      url = net::AppendQueryParameter(url, "basicLicenseToken", basic_token);
    }

    url = AppendAccountIdQueryParameter(url);
    return url;
  }
}  // namespace

LicenseOnlineFetcher::LicenseOnlineFetcher() = default;
/*
{
  net::URLRequestContextBuilder context_builder;
  context_builder.set_user_agent("FydeOS license Fetcher/1.0");
#if defined(OS_LINUX)
  context_builder.set_proxy_config_service(std::make_unique<net::ProxyConfigServiceFixed>(
            net::ProxyConfigWithAnnotation()));
#endif
  url_request_context_ = context_builder.Build();
  request_context_getter_ = new net::TrivialURLRequestContextGetter(url_request_context_.get(),
     content::GetIOThreadTaskRunner({}));
}
*/

LicenseOnlineFetcher::~LicenseOnlineFetcher() = default;

FetchMode LicenseOnlineFetcher::GetFetchMode() {
  return FetchMode::OnlineMode;
}

void LicenseOnlineFetcher::StartFetch(
    const std::string& id,
    const std::string& serial_number,
    const bool is_new_license,
    const std::string& oem_token,
    const std::string& basic_token,
    SuccessCallback<std::optional<std::string>> success_callback,
    ErrorCallback err_callback) {
  if (id.empty()) {
    std::move(err_callback).Run(-1, "License id is needed.");
    return;
  }
  success_callback_ = std::move(success_callback);
  err_callback_ = std::move(err_callback);
  /*
  auto fetcher = net::URLFetcher::Create(GetFullURL(id), net::URLFetcher::GET, this, kTrafficAnnotation);
  fetcher->SetRequestContext(request_context_getter_.get());
  VLOG(1) << "start fetch: " << GetFullURL(id);
  fetcher->Start();
  */
  auto resource_request = std::make_unique<network::ResourceRequest>();
  resource_request->credentials_mode = network::mojom::CredentialsMode::kOmit;
  resource_request->url = GetFullURL(id, serial_number, is_new_license, oem_token, basic_token);
  resource_request->method = "GET";
  simple_loader_ = network::SimpleURLLoader::Create(
      std::move(resource_request), kTrafficAnnotation);
  simple_loader_->SetAllowHttpErrorResults(true);
  simple_loader_->SetTimeoutDuration(base::Minutes(kFetcherTimeout));
  if (g_browser_process->system_network_context_manager()->HasInstance()) {
    network::mojom::URLLoaderFactory* loader_factory =
      g_browser_process->system_network_context_manager()
                       ->GetURLLoaderFactory();
    simple_loader_->DownloadToString(
      loader_factory,
      base::BindOnce(&LicenseOnlineFetcher::OnURLFetchComplete,
                     base::Unretained(this)),
      kMaxDownloadSize);
    return;
  }
  std::move(err_callback_).Run(-40, "system_network_context is not exist.");
}

void LicenseOnlineFetcher::OnTimeout() {
  std::move(err_callback_).Run(-30, "Online fetcher is timeout.");
}

void LicenseOnlineFetcher::OnURLFetchComplete(
    std::unique_ptr<std::string> response_body) {
  std::unique_ptr<network::SimpleURLLoader> url_loader =
    std::move(simple_loader_);
  int response_code = 0;
  if (url_loader->ResponseInfo() && url_loader->ResponseInfo()->headers)
    response_code = url_loader->ResponseInfo()->headers->response_code();
  VLOG(2) << "received code:" << response_code
          << " final url:" << url_loader->GetFinalURL();
  if (response_body)
    VLOG(2) << "received body:" << *response_body;
  if (response_code != net::HTTP_OK) {
    VLOG(2) << "net error:" << url_loader->NetError();
    std::move(err_callback_).Run(response_code, "Error get online license");
  } else {
    std::move(success_callback_).Run(*response_body);
  }
}

}  // namespace fydeos::license
