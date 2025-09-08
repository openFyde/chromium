// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ash/policy/reporting/metrics_reporting/kiosk_heartbeat/kiosk_heartbeat_telemetry_sampler.h"

#include "chrome/browser/ash/settings/device_settings_service.h"
#include "components/reporting/metrics/sampler.h"
#include "components/reporting/proto/synced/metric_data.pb.h"

#include "chrome/browser/browser_process.h"
#include "chrome/browser/net/system_network_context_manager.h"
#include "services/network/public/cpp/resource_request.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"
#include "net/traffic_annotation/network_traffic_annotation.h"
#include "net/http/http_status_code.h"
#include "services/network/public/mojom/url_response_head.mojom.h"
#include "base/json/json_writer.h"
#include "fydeos/build/config/buildflags.h"
#include "fydeos/switches/account/account_switches.h"
#include "fydeos/switches/account/account_constants.h"
#include "base/command_line.h"

namespace reporting {

namespace {

constexpr char kFydeosKioskHeartbeatServerPath[] =
  "/report/kioskHeartbeat";

std::string heartbeat_server_url() {
  std::string base_url;
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(fydeos::switches::kFydeOSDeviceManagementUrl)) {
    base_url = command_line->GetSwitchValueASCII(fydeos::switches::kFydeOSDeviceManagementUrl);
  } else {
    base_url = fydeos::constants::kDefaultFydeOSDeviceManagementServerUrl;
  }
  return base::StrCat({base_url, kFydeosKioskHeartbeatServerPath});
}

constexpr char kJsonContentType[] = "application/json";
constexpr int kHeartBeatRequestTimeoutInSeconds = 30;
constexpr size_t kMaxMessageSize = 1024;
constexpr net::NetworkTrafficAnnotationTag kTrafficAnnotation =
  net::DefineNetworkTrafficAnnotation("kiosk_fydeos_heartbeat", R"(
      semantics {
        sender: "FydeOS"
        description:
          "This request is used to send heartbeat request to fydeos server"
        trigger:
          "kiosk_heartbeat_telemetry_sampler"
        data: "None."
        destination: KIOSK_FYDEOS_HEARTBEAT_SERVER
      }
      policy {
        cookies_allowed: NO
        policy_exception_justification: "Not implemented."
      })");
}

void KioskHeartbeatTelemetrySampler::MaybeCollect(
    OptionalMetricCallback callback) {
  MetricData metric_data;
  // using mutable_.. to just add the empty heartbeat_telemetry message.
  metric_data.mutable_telemetry_data()->mutable_heartbeat_telemetry();

  std::move(callback).Run(std::move(metric_data));
}

void KioskHeartbeatTelemetrySampler::MaybeSendHeartBeatToFyde() {
  if (requesting_) {
    return;
  }
  if (!g_browser_process->shared_url_loader_factory()) {
    return;
  }
  const enterprise_management::PolicyData* const policy_data =
      ash::DeviceSettingsService::Get()->policy_data();
  if (!policy_data
    || !policy_data->has_request_token()
    || policy_data->request_token().empty()) {
    return;
  }
  base::Value::Dict post_body_value;
  post_body_value.Set("dm_token", policy_data->request_token());
  std::string post_body;
  bool write_success = base::JSONWriter::Write(post_body_value, &post_body);
  if (!write_success) {
    return;
  }
  requesting_ = true;
  auto resource_request = std::make_unique<network::ResourceRequest>();
  resource_request->credentials_mode = network::mojom::CredentialsMode::kOmit;
  resource_request->url = GURL(heartbeat_server_url());
  resource_request->method = "POST";
  simple_loader_ = network::SimpleURLLoader::Create(
      std::move(resource_request), kTrafficAnnotation);
  simple_loader_->AttachStringForUpload(post_body, kJsonContentType);
  simple_loader_->SetAllowHttpErrorResults(true);
  simple_loader_->SetTimeoutDuration(base::Seconds(kHeartBeatRequestTimeoutInSeconds));
  // int retry_mode = network::SimpleURLLoader::RETRY_ON_NETWORK_CHANGE
  //                  | network::SimpleURLLoader::RETRY_ON_NAME_NOT_RESOLVED;
  // simple_loader_->SetRetryOptions(3, retry_mode);

  auto loader_factory =
    g_browser_process->shared_url_loader_factory();
  simple_loader_->DownloadToString(
    loader_factory.get(),
    base::BindOnce(&KioskHeartbeatTelemetrySampler::OnURLFetchComplete,
                    base::Unretained(this)),
    kMaxMessageSize);
}

void KioskHeartbeatTelemetrySampler::OnURLFetchComplete(
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
  }
  requesting_ = false;
}
}  // namespace reporting
