// Copyright 2020 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FYDEOS_LICENSE_ENFORCEMENT_H_
#define FYDEOS_LICENSE_ENFORCEMENT_H_

#include <string>
#include <memory>

class Profile;

namespace base {
  class OneShotTimer;
  class RepeatingTimer;
}
namespace content {
  class WebContents;
}

namespace message_center{
  class Notification;
}

namespace fydeos {
namespace license {

enum EnforcementMode {
  EnforcementModeLevel0 = 0,
  EnforcementModeLevel1 = 1,
  EnforcementModeLevel2 = 2,
  // EnforcementModeLevel3 = 3,
};

const EnforcementMode EnforcementModeDefault = EnforcementModeLevel2;

class LicenseEnforcement {
  public:
    explicit LicenseEnforcement();
    ~LicenseEnforcement();

    void StartEnforcement(const std::string& licenseID, const std::string& serialNumber, EnforcementMode mode);
    void StopEnforcement();

  private:
    void Enforce();
    void PopupLicenseWindow();
    void CloseLicenseWindow();
    void ForceQuitCurrentUser();


    void ForceQuitNotification();
    void KickOut();

    void OnForceQuitNotificationClicked();
    void RemoveForceQuitNotification();
    void UpdateForceQuitNotification();
    std::u16string ForceQuitNotificationMessage();
    void PopupForceQuitNotification();

    std::unique_ptr<base::RepeatingTimer> enforce_timer_;
    std::unique_ptr<base::OneShotTimer> force_quit_timer_;
    std::unique_ptr<base::RepeatingTimer> notification_timer_;
    std::unique_ptr<message_center::Notification> notification_;
    content::WebContents* webContents_;
    Profile* profile_;
    std::string id_;
    std::string serial_number_;
    EnforcementMode mode_;
}; // class LicenseEnforcement

} // namespace license

} // namespace fydeos
#endif
