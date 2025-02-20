// Copyright 2020 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FYDEOS_LICENSE_MANAGER_H_
#define FYDEOS_LICENSE_MANAGER_H_
#include <memory>
#include <string>
#include "base/timer/timer.h"
#include "base/threading/thread.h"
#include "fydeos/chromeos/ash/components/dbus/fydeos_shell_client/shell_state.h"
#include "fydeos/license/fydeos_license_fetcher.h"
#include "fydeos/license/fydeos_license_validator.h"
#include "fydeos/license/fydeos_license_enforcement.h"
#include "chromeos/chromeos_export.h"
#include "chromeos/ash/components/login/login_state/login_state.h"

namespace fydeos::license {

enum ManagerState {
  Idle,
  Running,
};

class CHROMEOS_EXPORT LicenseManager : public ::ash::LoginState::Observer {
 public:
  class Observer {
   public:
    // Called when either the login state or the logged in user type changes.
    virtual void OnLicenseStateChanged(bool IsValid) = 0;
   protected:
    virtual ~Observer() = default;
  };
  LicenseManager();
  ~LicenseManager() override;
  static void Initialize();
  static LicenseManager* Get();
  static void Shutdown();
  void Start();
  void Stop();
  void CheckLicense();
  ManagerState GetState();
  bool IsValid();
  // Add/remove observers.
  void AddObserver(Observer* observer);
  void RemoveObserver(Observer* observer);

 private:
  void Reset();
  void IFetchOnlineLicense();
  void OnFetchedOnlineLicense(std::string license);
  void IFetchOfflineLicense();
  void OnFetchedOfflineLicense(std::optional<fydeos::ash::ShellState> state);
  void IGetId();
  void IGetSerialNumber();
  void NotifyObservers();
  void OnGotId(std::optional<fydeos::ash::ShellState> state);
  void OnGotSerialNumber(std::optional<fydeos::ash::ShellState> state);
  void IValidateLicense(std::optional<std::string> license);
  void ISaveLocalLicenseString(std::optional<base::Value> license);
  void IStoreLicense(std::optional<base::Value> license);
  void OnStoredLicense(std::optional<fydeos::ash::ShellState> state);
  void OnError(int errCode, const std::string& errMsg);
  void OnInvalid();
  void OnValid(std::optional<base::Value> license);
  void SkipOnline();
  void LoggedInStateChanged() override;

  ManagerState state_ = ManagerState::Idle;
  FetchMode mode_;
  bool is_valid_ = false;
  bool is_new_license_ = false;
  std::string id_;
  std::string serial_number_;
  std::string license_string;
  std::unique_ptr<base::RepeatingTimer> timer_;
  std::unique_ptr<LicenseOnlineFetcher> online_fetcher_;
  std::unique_ptr<LicenseValidator> validator_;
  std::unique_ptr<LicenseEnforcement> enforcement_;
  base::ObserverList<Observer>::Unchecked observer_list_;
};

}  // namespace fydeos::license


#endif  // FYDEOS_LICENSE_MANAGER_H_
