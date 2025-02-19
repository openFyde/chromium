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
const char kFydeOSLicenseWebUrl[] = "fydeos-license-web-url";
const char kFydeOSLicenseApiUrl[] = "fydeos-license-api-url";

}

bool IsLicenseTestMode() {
  return base::CommandLine::ForCurrentProcess()->HasSwitch(kLicenseTestMode);
}

std::string GetFydeOSLicenseWebUrl() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(kFydeOSLicenseWebUrl)) {
    return command_line->GetSwitchValueASCII(kFydeOSLicenseWebUrl);
  } else {
    return std::string(fydeos::constants::kDefaultFydeOSLicenseWebUrl);
  }
}

std::string GetFydeOSLicenseApiUrl() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(kFydeOSLicenseApiUrl)) {
    return command_line->GetSwitchValueASCII(kFydeOSLicenseApiUrl);
  } else {
    return std::string(fydeos::constants::kDefaultFydeOSLicenseApiUrl);
  }
}

} // switches
} // fydeos
