// Copyright (c) 2019 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fydeos/switches/features/network_switches.h"
#include "base/command_line.h"

namespace fydeos {
namespace switches {

namespace {

const char kResetWifiDriver[] = "fydeos-reset-wifi-driver";

}

bool NeedResetWifiDriver() {
  return base::CommandLine::ForCurrentProcess()->HasSwitch(kResetWifiDriver);
}

} // switches
} // fydeos

