// Copyright (c) 2019 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fydeos/switches/license/license_switches.h"
#include "base/command_line.h"
#include "fydeos/switches/license/license_constants.h"

namespace fydeos {
namespace switches {

namespace {

const char kLicenseTestMode[] = "fydeos-license-test-mode";
const char kFydeOSLicenseUrl[] = "fydeos-license-url";

}

bool IsLicenseTestMode() {
  return base::CommandLine::ForCurrentProcess()->HasSwitch(kLicenseTestMode);
}

std::string GetFydeOSLicenseUrl() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(kFydeOSLicenseUrl)) {
    return command_line->GetSwitchValueASCII(kFydeOSLicenseUrl);
  } else {
    return std::string(fydeos::constants::kDefaultFydeOSLicenseUrl);
  }
}

} // switches
} // fydeos
