// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FYDEOS_STATISTICS_COLLECTOR_H
#define FYDEOS_STATISTICS_COLLECTOR_H

#include <memory>
#include <string>
#include "third_party/abseil-cpp/absl/types/optional.h"
#include "chromeos/chromeos_export.h"
#include "base/memory/weak_ptr.h"
#include "base/time/time.h"
#include "base/functional/callback.h"
#include "chromeos/ash/components/login/login_state/login_state.h"
#include "fydeos/chromeos/ash/components/dbus/fydeos_shell_client/shell_state.h"

class Profile;

namespace chromeos {
namespace system {
class StatisticsProvider;
}
}

namespace network {
class SimpleURLLoader;
}

namespace fydeos {
namespace misc {

class StatisticsCollector {
 public:
    StatisticsCollector();
    ~StatisticsCollector();

    void Start();
    void Stop();

 private:
    struct Statistics {
      std::string license_id;
      std::string license_type;
      std::string major_version;
      std::string board_name;
      std::string hardware_model_name;
      std::string region;
      std::string ethernet_mac_address;
      std::string dock_mac_address;
      std::string sn;
      std::string os_version;
      std::string kernel_version;
      base::Time os_release_time;
      std::string channel_name;
      std::string browser_version;
      std::string browser_milestone;
      std::string browser_language;
      bool is_multi_boot;

      std::string profile_account_id;
      std::string profile_account_type;
      base::Time profile_creation_time;
      base::Time profile_start_time;
      bool is_new_profile;
      bool is_fyde_profile;
    };

    enum CollectStep {
      INITIALIZE = 0,
      GET_LICENSE_ID,
      GET_LICENSE_TYPE,
      GET_INSTALL_TYPE,
      SYNC_COLLECT,
    };

    void StartInternal();
    void OnMachineStatisticsLoaded();
    void ProceedToNextStep();

    void GetLicenseId();
    void OnGetLicenseId(absl::optional<fydeos::ash::ShellState> state);

    void GetLicenseType();
    void OnGetLicenseType(absl::optional<fydeos::ash::ShellState> state);
    void GetLicenseTypeFromInfo(const std::string& license);

    void GetInstallType();
    void OnGetInstallType(absl::optional<fydeos::ash::ShellState> state);

    void SyncCollect();

    void CollectUserInfo();
    void CollectDeviceInfo();

    bool UploadStatistics();
    bool GetUploadData(std::string* output);
    bool EncryptData(const std::string& text, std::string* encrypted);
    void OnUploaded(std::unique_ptr<network::SimpleURLLoader> url_loader,
                    std::unique_ptr<std::string> response_body);

    bool started_ = false;
    CollectStep step_;
    Statistics statistics_ = {};
    Profile* profile_;

    base::WeakPtrFactory<StatisticsCollector> weak_factory_{this};
};

}  // namespace misc
}  // namespace fydeos

#endif /* ifndef FYDEOS_STATISTICS_COLLECTOR_H */

