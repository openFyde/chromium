// Copyright (c) 2022 Fyde Innovations. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fydeos/switches/account/toggle/account_type_toggle.h"
#include "fydeos/switches/account/account_switches.h"

#include "google_apis/gaia/gaia_urls.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/policy/chrome_browser_policy_connector.h"
#include "chrome/browser/browser_process_platform_part_chromeos.h"
#include "components/user_manager/user_manager.h"
#include "components/account_id/account_id.h"
#include "base/command_line.h"

namespace fydeos {
namespace switches {

namespace {
bool is_device_managed = false;

void ResetUrls() {
  GaiaUrls *gaia_urls = GaiaUrls::GetInstance();
  if (gaia_urls) {
    gaia_urls->Reset();
  }

  if (!g_browser_process || !g_browser_process->platform_part()) return;
  policy::ChromeBrowserPolicyConnector* connector =
      g_browser_process->browser_policy_connector();
  if (connector) {
    connector->ResetDeviceManagementServiceConfiguration();
  }
}

void ToggleFydeAccountFlagInternal(base::CommandLine* cmdline, const AccountType account_type) {
  if (cmdline->HasSwitch(kFydeAccountForceDisabledForTest)) return;
  if (is_device_managed) return;
  bool need_reset_url = false;
  if (account_type == AccountType::FYDE_ACCOUNT) {
    if (!cmdline->HasSwitch(kFydeAccountEnable)) {
      cmdline->AppendSwitch(kFydeAccountEnable);
      need_reset_url = true;
    }
  } else if (account_type == AccountType::GOOGLE) {
    if (cmdline->HasSwitch(kFydeAccountEnable)) {
      cmdline->RemoveSwitch(kFydeAccountEnable);
      need_reset_url = true;
    }
  } else {
    return;
  }

  if (need_reset_url) {
    ResetUrls();
  }
}

user_manager::User* GetActiveUserInternal() {
  if (!user_manager::UserManager::IsInitialized()) { return nullptr; }
  user_manager::UserManager* user_manager_ = user_manager::UserManager::Get();
  if (!user_manager_) { return nullptr; }
  user_manager::User* user = user_manager_->GetActiveUser();
  return user;
}

}

void ToggleFydeAccountFlagByAccountId(const AccountId& account_id) {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  ToggleFydeAccountFlagInternal(command_line, account_id.GetAccountType());
}

void ToggleFydeAccountFlagForCommandLine(base::CommandLine* command_line) {
  user_manager::User* user = GetActiveUserInternal();
  if (!user) return;
  AccountId account_id = user->GetAccountId();
  ToggleFydeAccountFlagInternal(command_line, account_id.GetAccountType());
}

void ToggleFydeAccountFlagForCommandLineByAccountId(base::CommandLine* command_line, const AccountId& account_id) {
  ToggleFydeAccountFlagInternal(command_line, account_id.GetAccountType());
}

void EnableFydeAccountFlag() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  ToggleFydeAccountFlagInternal(command_line, AccountType::FYDE_ACCOUNT);
}

void DisableFydeAccountFlag() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  ToggleFydeAccountFlagInternal(command_line, AccountType::GOOGLE);
}

void ToggleFydeAccountFlagByActiveUser() {
  user_manager::User* user = GetActiveUserInternal();
  if (!user) return;
  ToggleFydeAccountFlagByAccountId(user->GetAccountId());
}

void EnableFydeAccountFlagForManagedDevice() {
  EnableFydeAccountFlag();
  is_device_managed = true;
}

void DisableFydeAccountFlagForManagedDevice() {
  DisableFydeAccountFlag();
  is_device_managed = true;
}

void SetDeviceManagedFlag(bool is_managed) {
  is_device_managed = is_managed;
}

void AppendAccountSwitchesIfNeed(const AccountId& account_id, std::vector<std::string>* switches) {
  // GetSwitchString
  base::CommandLine cmd_line(base::CommandLine::NO_PROGRAM);
  cmd_line.AppendSwitch(kFydeAccountEnable);
  const std::string account_switch = cmd_line.argv()[1];

  if (account_id.GetAccountType() == AccountType::FYDE_ACCOUNT && std::find(switches->begin(), switches->end(), account_switch) == switches->end()) {
    switches->push_back(account_switch);
  }
}

} // namespace switches
} // namespace fydeos
