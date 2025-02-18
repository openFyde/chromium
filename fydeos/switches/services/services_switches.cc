// Copyright (c) 2019 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fydeos/switches/services/services_switches.h"

#include "base/command_line.h"
#include "fydeos/switches/services/services_constants.h"

namespace fydeos {
namespace switches {

namespace {

const char kDisableFydeOSGeolocationAPI[] = "disable-fydeos-geolocation-api";
const char kDisableFydeOSTimezoneAPI[] = "disable-fydeos-timezone-api";

const char kFydeOSGeolocationAPIUrl[] = "fydeos-geolocation-api-url";
const char kFydeOSTimezoneAPIUrl[] = "fydeos-timezone-api-url";

}

bool DisableFydeOSGeolocationAPI() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  return command_line->HasSwitch(kDisableFydeOSGeolocationAPI);
}

bool DisableFydeOSTimezoneAPI() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  return command_line->HasSwitch(kDisableFydeOSTimezoneAPI);
}

std::string GetFydeOSGeolocationAPIUrl() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(kFydeOSGeolocationAPIUrl)) {
    return command_line->GetSwitchValueASCII(kFydeOSGeolocationAPIUrl);
  } else {
    return std::string(fydeos::constants::kDefaultFydeOSGeolocationAPIUrl);
  }
}

std::string GetFydeOSTimezoneAPIUrl() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(kFydeOSTimezoneAPIUrl)) {
    return command_line->GetSwitchValueASCII(kFydeOSTimezoneAPIUrl);
  } else {
    return std::string(fydeos::constants::kDefaultFydeOSTimezoneAPIUrl);
  }
}

} // switches
} // fydeos
