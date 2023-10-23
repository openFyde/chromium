// Copyright (c) 2019 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fydeos/switches/license/license_constants.h"
#include "fydeos/build/config/buildflags.h"

namespace fydeos::constants {

#if BUILDFLAG(USE_FYDEOS_COM)
const char kDefaultFydeOSLicenseUrl[] = "https://cashier.fydeos.com";
#else
const char kDefaultFydeOSLicenseUrl[] = "https://cashier.fydeos.io";
#endif

}  // namespace fydeos::constants
