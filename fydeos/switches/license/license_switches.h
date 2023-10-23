// Copyright (c) 2019 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_FYDEOS_SWITCHES_LICENSE_SWITCHES_H_
#define CHROMEOS_FYDEOS_SWITCHES_LICENSE_SWITCHES_H_

#include "chromeos/chromeos_export.h"
#include <string>

namespace fydeos {
namespace switches {

extern std::string GetFydeOSLicenseUrl();
CHROMEOS_EXPORT bool IsLicenseTestMode();

} // switches
} // fydeos

#endif
