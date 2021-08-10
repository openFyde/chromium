// Copyright (c) 2019 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fydeos/switches/account/account_switches.h"
#include "base/command_line.h"

namespace fydeos {
namespace switches {

namespace {

const char kFydeAccountEnable[] = "fyde-account-enabled";
const char kFydeAccountForceDisabledForTest[] = "fyde-account-force-disabled";

}

bool IsFydeAccountEnabled() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  return command_line->HasSwitch(kFydeAccountEnable) && !command_line->HasSwitch(kFydeAccountForceDisabledForTest);
}

}// namespace switches
}// namespace fydeos
