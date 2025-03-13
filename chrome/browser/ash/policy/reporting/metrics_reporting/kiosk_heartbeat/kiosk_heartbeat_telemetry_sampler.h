// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_ASH_POLICY_REPORTING_METRICS_REPORTING_KIOSK_HEARTBEAT_KIOSK_HEARTBEAT_TELEMETRY_SAMPLER_H_
#define CHROME_BROWSER_ASH_POLICY_REPORTING_METRICS_REPORTING_KIOSK_HEARTBEAT_KIOSK_HEARTBEAT_TELEMETRY_SAMPLER_H_

#include "components/reporting/metrics/sampler.h"
#include "services/network/public/cpp/simple_url_loader.h"

namespace reporting {
// Sampler used to create KioskHeartbeat messages to be sent via ERP controlled
// by a KioskHeartbeatTelemetryCollector.
class KioskHeartbeatTelemetrySampler : public Sampler {
 public:
  KioskHeartbeatTelemetrySampler() = default;
  KioskHeartbeatTelemetrySampler(const KioskHeartbeatTelemetrySampler& other) =
      delete;
  KioskHeartbeatTelemetrySampler& operator=(
      const KioskHeartbeatTelemetrySampler& other) = delete;
  ~KioskHeartbeatTelemetrySampler() override = default;

  // Sends KioskHeartbeats whenever called and passes it to the callback.
  void MaybeCollect(OptionalMetricCallback callback) override;

  void MaybeSendHeartBeatToFyde() override;

 private:
  void OnURLFetchComplete(std::unique_ptr<std::string> response_body);
  void OnTimeout();

  std::unique_ptr<network::SimpleURLLoader> simple_loader_;

  bool requesting_ = false;

};
}  // namespace reporting

#endif  // CHROME_BROWSER_ASH_POLICY_REPORTING_METRICS_REPORTING_KIOSK_HEARTBEAT_KIOSK_HEARTBEAT_TELEMETRY_SAMPLER_H_
