// Copyright (c) 2019 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fydeos/switches/account/account_switches.h"
#include "fydeos/switches/account/account_constants.h"
#include "base/command_line.h"

#include "google_apis/gaia/gaia_urls.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/ash/policy/core/browser_policy_connector_ash.h"
#include "chrome/browser/browser_process_platform_part_chromeos.h"
#include "components/user_manager/user_manager.h"
#include "components/account_id/account_id.h"

namespace fydeos {
namespace switches {

namespace {

const char kFydeAccountEnable[] = "fyde-account-enabled";
const char kFydeAccountForceDisabledForTest[] = "fyde-account-force-disabled";

const char kFydeOSFamilyLinkApisUrl[] = "fydeos-family-link-apis-url";
const char kFydeOSClassifyURLRequestApiPath[] = "fydeos-classify-url-request-api-path";
const char kFydeOSSupervisedUserSettingsSyncIntervalInSeconds[] = "fydeos-supervised-user-settings-sync-interval";

const char kPolicyManagedByFyde[] = "policy-managed-by-fyde";

bool is_device_managed = false;

void ResetUrls() {
  GaiaUrls *gaia_urls = GaiaUrls::GetInstance();
  if (gaia_urls) {
    gaia_urls->Reset();
  }

  if (!g_browser_process || !g_browser_process->platform_part()) return;
  policy::BrowserPolicyConnectorAsh* connector =
      g_browser_process->platform_part()->browser_policy_connector_ash();
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
  if (!user_manager::UserManager::IsInitialized()) { return NULL; }
  user_manager::UserManager* user_manager_ = user_manager::UserManager::Get();
  if (!user_manager_) { return NULL; }
  user_manager::User* user = user_manager_->GetActiveUser();
  return user;
}

}

bool IsFydeAccountEnabled() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  return command_line->HasSwitch(kFydeAccountEnable) && !command_line->HasSwitch(kFydeAccountForceDisabledForTest);
}

const char kFydeOSGaiaUrl[] = "fydeos-gaia-url";
const char kFydeOSApisUrl[] = "fydeos-apis-url";
const char kFydeOSDeviceManagementUrl[] = "fydeos-device-management-url";
const char kFydeOSRealtimeReportingUrl[] = "fydeos-realtime-reporting-url";
const char kFydeOSEncryptedReportingUrl[] = "fydeos-encrypted-reporting-url";
const char kFydeOSSyncServiceURL[] = "fydeos-sync-url";

std::string GetFydeOSKidsManagementAPIBaseUrl() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(kFydeOSFamilyLinkApisUrl)) {
    return command_line->GetSwitchValueASCII(kFydeOSFamilyLinkApisUrl);
  } else {
    return std::string(fydeos::constants::kDefaultFydeOSFamilyLinkApisUrl);
  }
}

std::string GetClassifyURLRequestApiPath() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(kFydeOSClassifyURLRequestApiPath)) {
    return command_line->GetSwitchValueASCII(kFydeOSClassifyURLRequestApiPath);
  } else {
    return std::string(fydeos::constants::kDefaultFydeOSClassifyURLRequestApiPath);
  }
}

int GetFydeOSSupervisedUserSettingsSyncInterval() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  int interval = 0;
  if (command_line->HasSwitch(kFydeOSSupervisedUserSettingsSyncIntervalInSeconds)) {
    std::string str = command_line->GetSwitchValueASCII(kFydeOSSupervisedUserSettingsSyncIntervalInSeconds);
    interval = std::stoi(str);
  }
  return interval < 10 ? fydeos::constants::kFydeOSSupervisedUserSettingsDefaultSyncIntervalInSeconds : interval;
}

bool IsPolicyManagedByFyde() {
  return base::CommandLine::ForCurrentProcess()->HasSwitch(kPolicyManagedByFyde) || IsFydeAccountEnabled();
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

}// namespace switches
}// namespace fydeos
