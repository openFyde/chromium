// Copyright (c) 2019 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_FYDEOS_SWITCHES_ACCOUNT_SWITCHES_H_
#define CHROMEOS_FYDEOS_SWITCHES_ACCOUNT_SWITCHES_H_

#include <string>
#include "chromeos/chromeos_export.h"

namespace base {
  class CommandLine;
}
class AccountId;

namespace fydeos {
namespace switches {

CHROMEOS_EXPORT bool IsFydeAccountEnabled();
CHROMEOS_EXPORT bool IsFydeExtendAccountEnabled();

CHROMEOS_EXPORT extern const char kFydeOSGaiaUrl[];
CHROMEOS_EXPORT extern const char kFydeOSApisUrl[];
CHROMEOS_EXPORT extern const char kFydeOSDeviceManagementUrl[];
CHROMEOS_EXPORT extern const char kFydeOSRealtimeReportingUrl[];
CHROMEOS_EXPORT extern const char kFydeOSEncryptedReportingUrl[];
CHROMEOS_EXPORT extern const char kFydeOSSyncServiceURL[];
extern std::string GetFydeOSKidsManagementAPIBaseUrl();
extern std::string GetClassifyURLRequestApiPath();
CHROMEOS_EXPORT int GetFydeOSSupervisedUserSettingsSyncInterval();
CHROMEOS_EXPORT bool IsPolicyManagedByFyde();

CHROMEOS_EXPORT void EnableFydeAccountFlag();
CHROMEOS_EXPORT void DisableFydeAccountFlag();
CHROMEOS_EXPORT void ToggleFydeAccountFlagByAccountId(const AccountId& account_id);
CHROMEOS_EXPORT void ToggleFydeAccountFlagForCommandLine(base::CommandLine* command_line);
CHROMEOS_EXPORT void ToggleFydeAccountFlagForCommandLineByAccountId(base::CommandLine* command_line, const AccountId& account_id);
CHROMEOS_EXPORT void ToggleFydeAccountFlagByActiveUser();
CHROMEOS_EXPORT void EnableFydeAccountFlagForManagedDevice();
CHROMEOS_EXPORT void DisableFydeAccountFlagForManagedDevice();
CHROMEOS_EXPORT void AppendAccountSwitchesIfNeed(const AccountId& account_id, std::vector<std::string>* switches);

}
}

#endif
