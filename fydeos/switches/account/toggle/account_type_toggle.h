// Copyright (c) 2022 Fyde Innovations. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FYDEOS_SWITCHES_ACCOUNT_TOGGLE_ACCOUNT_TYPE_TOGGLE_H_
#define FYDEOS_SWITCHES_ACCOUNT_TOGGLE_ACCOUNT_TYPE_TOGGLE_H_

#include "chromeos/chromeos_export.h"
#include <string>

namespace base {
  class CommandLine;
}
class AccountId;

namespace fydeos {
namespace switches {

CHROMEOS_EXPORT void EnableFydeAccountFlag();
CHROMEOS_EXPORT void DisableFydeAccountFlag();
CHROMEOS_EXPORT void ToggleFydeAccountFlagByAccountId(const AccountId& account_id);
CHROMEOS_EXPORT void ToggleFydeAccountFlagForCommandLine(base::CommandLine* command_line);
CHROMEOS_EXPORT void ToggleFydeAccountFlagForCommandLineByAccountId(base::CommandLine* command_line, const AccountId& account_id);
CHROMEOS_EXPORT void ToggleFydeAccountFlagByActiveUser();
CHROMEOS_EXPORT void EnableFydeAccountFlagForManagedDevice();
CHROMEOS_EXPORT void DisableFydeAccountFlagForManagedDevice();
CHROMEOS_EXPORT void SetDeviceManagedFlag(bool is_managed);
CHROMEOS_EXPORT void AppendAccountSwitchesIfNeed(const AccountId& account_id, std::vector<std::string>* switches);


} // namespace switches
} // namespace fydeos


#endif  // FYDEOS_SWITCHES_ACCOUNT_TOGGLE_ACCOUNT_TYPE_TOGGLE_H_
