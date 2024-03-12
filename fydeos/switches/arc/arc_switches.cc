// Copyright (c) 2019 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fydeos/switches/arc/arc_switches.h"
#include "base/command_line.h"
#include <string>

namespace fydeos {
namespace switches {

namespace {

const char kFydeOSArcDelay[] ="fydeos-arc-delay";

}

int64_t GetFydeOSArcDelay() {
	std::string delayStr = base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(kFydeOSArcDelay);
	if (delayStr.empty())
		return 0;
	return (int64_t) std::stoi(delayStr);
}

} // switches
} // fydeos

