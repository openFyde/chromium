// Copyright (c) 2019 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fydeos/switches/account/account_switches.h"
#include "fydeos/switches/account/account_constants.h"
#include "base/command_line.h"

namespace fydeos {
namespace switches {

namespace {

const char kFydeOSFamilyLinkApisUrl[] = "fydeos-family-link-apis-url";
const char kFydeOSClassifyURLRequestApiPath[] = "fydeos-classify-url-request-api-path";
const char kFydeOSSupervisedUserSettingsSyncIntervalInSeconds[] = "fydeos-supervised-user-settings-sync-interval";

const char kPolicyManagedByFyde[] = "policy-managed-by-fyde";

}

bool IsFydeAccountEnabled() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  return command_line->HasSwitch(kFydeAccountEnable) && !command_line->HasSwitch(kFydeAccountForceDisabledForTest);
}

const char kFydeAccountEnable[] = "fyde-account-enabled";
const char kFydeAccountForceDisabledForTest[] = "fyde-account-force-disabled";

bool IsFydeExtendAccountEnabled() {
  return IsFydeAccountEnabled();
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

}// namespace switches
}// namespace fydeos
