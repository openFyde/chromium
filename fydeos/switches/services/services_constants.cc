// Copyright (c) 2019 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fydeos/switches/services/services_constants.h"
#include "fydeos/build/config/buildflags.h"
#include "fydeos/switches/account/policy_constants.h"

namespace fydeos::constants {

#if BUILDFLAG(USE_FYDEOS_COM)
const char kDefaultFydeOSGeolocationAPIUrl[] =
  "https://apis.fydeos.com/geo/locationByIp?";
const char kDefaultFydeOSTimezoneAPIUrl[] =
  "https://apis.fydeos.com/geo/timezone?";
const char kDefaultFydeOSLookingGlassUrl[] =
  "https://apis.fydeos.com/lookingglass";
#else
const char kDefaultFydeOSGeolocationAPIUrl[] =
  "https://apis.fydeos.io/geo/locationByIp?";
const char kDefaultFydeOSTimezoneAPIUrl[] =
  "https://apis.fydeos.io/geo/timezone?";
const char kDefaultFydeOSLookingGlassUrl[] =
  "https://apis.fydeos.io/lookingglass";
#endif

const uint8_t* kFydeOSCryptoKey =
  fydeos::constants::kFydeOSPolicyVerificationKey;
const size_t kFydeOSCryptoKeyLength =
  fydeos::constants::kFydeOSPolicyVerificationKeyLength;

}  // namespace fydeos::constants
