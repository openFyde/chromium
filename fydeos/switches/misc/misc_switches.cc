// Copyright (c) 2019 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fydeos/switches/misc/misc_switches.h"
#include "base/command_line.h"

namespace fydeos {
namespace switches {

namespace {

const char kFydeDisableCustom[] = "fyde-disable-custom";

const char kEnableTpmDictionaryAttackLockout[] = "fydeos-enable-tpm-da-lockout";

}

bool IsFydeCustomEnabled() {
  return !base::CommandLine::ForCurrentProcess()->HasSwitch(kFydeDisableCustom);
}

bool IsTpmDictionaryAttackLockoutIgnored() {
  return !base::CommandLine::ForCurrentProcess()->HasSwitch(kEnableTpmDictionaryAttackLockout);
}



} // switches
} // fydeos
