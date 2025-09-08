// Copyright (c) 2019 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fydeos/switches/misc/misc_switches.h"
#include "fydeos/switches/account/account_switches.h"
#include "base/command_line.h"
#include "base/strings/string_util.h"

namespace fydeos {
namespace switches {

namespace {

const char kFydeDisableCustom[] = "fyde-disable-custom";

const char kEnableTpmDictionaryAttackLockout[] = "fydeos-enable-tpm-da-lockout";

const char kDisallowInitDevicePolicyWithoutStateKeys[] = "disallow-init-device-policy-without-state-keys";

const char kDisableUnknownPeripheralBatteryNotification[] = "disable-unknown-peripheral-battery-notification";

const char kNoUseFydeInvalidationService[] = "no-use-fyde-invalidation-service";

const char kFydeEnableDynamicDefaultWallpaper[] =
  "fyde-dynamic-default-wallpaper";

const std::vector<std::string> kNonForYouBoards = {
  "amd64-fydeos",
  "amd64-openfyde",
  "amd64-vmware",
  "amd64-generic",
  "fydetab_duo-fydeos",
};

const char kFydeAutoSigninDelay[] = "fyde-auto-signin-delay";

}

const char kFydeOSServiceHostSuffixForTesting[] =
  "fydeos-service-host-suffix-for-testing";

bool IsFydeCustomEnabled() {
  return !base::CommandLine::ForCurrentProcess()->HasSwitch(kFydeDisableCustom);
}

bool IsTpmDictionaryAttackLockoutIgnored() {
  return !base::CommandLine::ForCurrentProcess()->HasSwitch(kEnableTpmDictionaryAttackLockout);
}

bool IsNonForYouBoard(const std::string& board) {
  for (const auto& non_for_you_board : kNonForYouBoards) {
    if (non_for_you_board == board) {
      return true;
    }
    if (base::StartsWith(board, non_for_you_board)) {
      return true;
    }
  }
  return false;
}

bool IsInitDevicePolicyWithoutStateKeysAllowed() {
  return !base::CommandLine::ForCurrentProcess()->HasSwitch(kDisallowInitDevicePolicyWithoutStateKeys);
}

bool IsUnknownPeripheralBatteryNotificationDisabled() {
  return base::CommandLine::ForCurrentProcess()->HasSwitch(kDisableUnknownPeripheralBatteryNotification);
}

bool UseFydeInvalidationService() {
  return !base::CommandLine::ForCurrentProcess()->HasSwitch(kNoUseFydeInvalidationService) && !IsBasicLicenseDevice();
}

bool IsDynamicDefaultWallpaperSupported() {
  return base::CommandLine::ForCurrentProcess()->HasSwitch(
      kFydeEnableDynamicDefaultWallpaper);
}

int64_t GetFydeOSAutoSigninDelay() {
	std::string delayStr = base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(kFydeAutoSigninDelay);
	if (delayStr.empty())
		return 0;
	return (int64_t) std::stoi(delayStr);
}

} // switches
} // fydeos
