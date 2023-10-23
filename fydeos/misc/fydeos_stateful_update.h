// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FYDEOS_STATEFUL_UPDATE_H
#define FYDEOS_STATEFUL_UPDATE_H

#include <string>
#include <memory>

#include "third_party/abseil-cpp/absl/types/optional.h"
#include "chromeos/chromeos_export.h"
#include "chromeos/ash/components/login/login_state/login_state.h"
#include "fydeos/chromeos/ash/components/dbus/fydeos_shell_client/shell_state.h"

class Profile;

namespace base {
class OneShotTimer;
class RepeatingTimer;
}

namespace message_center {
class Notification;
}

namespace fydeos {

namespace misc {

class StatefulUpdater {
 public:
    StatefulUpdater();
    ~StatefulUpdater();
    void Start();
    void Stop();

 private:
    void CheckUpdateDelay();
    void CheckUpdate();
    void OnCheckUpdateDone(absl::optional<fydeos::ash::ShellState> state);
    void GenerateUpdateNotification();
    void CloseUpdateNotification();
    void DoUpdate();
    void OnDoUpdateStarted(absl::optional<fydeos::ash::ShellState> state);
    void GenerateProgressNotification();
    void CloseProgressNotification();
    void GenerateRebootNotification();
    void TrytoUpdateProgressNotification(std::string result);
    void CloseRebootNotification();
    void CheckUpdateStatus();
    void OnCheckUpdateStatusDone(absl::optional<fydeos::ash::ShellState> state);
    void Reboot();

    void HandleUpdateNotificationClick(absl::optional<int> button_index);
    void HandleRebootNotificationClick(absl::optional<int> button_index);

    Profile* profile_ = nullptr;
    bool started_ = false;
    std::unique_ptr<base::OneShotTimer> timer_;
    std::unique_ptr<base::RepeatingTimer> status_timer_;
    std::unique_ptr<message_center::Notification> progress_notification_;
    int check_update_count_;
    int check_status_count_;


    base::WeakPtrFactory<StatefulUpdater> weak_ptr_factory_{this};
};

}  // namespace misc

}  // namespace fydeos


#endif
