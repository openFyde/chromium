// Copyright (c) 2019 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_FYDEOS_SWITCHES_MISC_SWITCHES_H_
#define CHROMEOS_FYDEOS_SWITCHES_MISC_SWITCHES_H_

#include <string>
#include "chromeos/chromeos_export.h"

namespace fydeos {
namespace switches {

CHROMEOS_EXPORT bool IsFydeCustomEnabled();

CHROMEOS_EXPORT bool IsTpmDictionaryAttackLockoutIgnored();

CHROMEOS_EXPORT bool IsNonForYouBoard(const std::string& board);

CHROMEOS_EXPORT bool IsInitDevicePolicyWithoutStateKeysAllowed();

CHROMEOS_EXPORT bool IsUnknownPeripheralBatteryNotificationDisabled();

CHROMEOS_EXPORT bool UseFydeInvalidationService();

CHROMEOS_EXPORT bool IsDynamicDefaultWallpaperSupported();

CHROMEOS_EXPORT extern const char kFydeOSServiceHostSuffixForTesting[];

CHROMEOS_EXPORT int64_t GetFydeOSAutoSigninDelay();

} // switches
} // fydeos

#endif
