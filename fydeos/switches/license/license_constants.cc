// Copyright (c) 2019 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fydeos/switches/license/license_constants.h"
#include "fydeos/build/config/buildflags.h"

namespace fydeos::constants {

#if BUILDFLAG(USE_FYDEOS_COM)
const char kDefaultFydeOSLicenseWebUrl[] = "https://cashier.fydeos.com";
const char kDefaultFydeOSLicenseApiUrl[] = "https://apis.fydeos.com/license";
#else
const char kDefaultFydeOSLicenseWebUrl[] = "https://cashier.fydeos.io";
const char kDefaultFydeOSLicenseApiUrl[] = "https://apis.fydeos.io/license";
#endif

const char kFydeOSOEMTokenFilePath[] = "/usr/share/oem/fydeos_oem_token";

}  // namespace fydeos::constants
