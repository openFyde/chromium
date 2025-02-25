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
const char kFydeOSWebStoreUpdateURL[] = "https://store.fydeos.com";
const char kFydeOSFeedbackPostUrl[] = "https://apis.fydeos.com/feedback/submit";

const char kFydeOSAssistantDefaultWebUrl[] = "https://ai.fydeos.com/chat/";
#else
const char kDefaultFydeOSGeolocationAPIUrl[] =
  "https://apis.fydeos.io/geo/locationByIp?";
const char kDefaultFydeOSTimezoneAPIUrl[] =
  "https://apis.fydeos.io/geo/timezone?";
const char kDefaultFydeOSLookingGlassUrl[] =
  "https://apis.fydeos.io/lookingglass";
const char kFydeOSWebStoreUpdateURL[] = "https://store.fydeos.io";
const char kFydeOSFeedbackPostUrl[] = "https://apis.fydeos.io/feedback/submit";

const char kFydeOSAssistantDefaultWebUrl[] = "https://ai.fydeos.io/chat/";
#endif

const uint8_t* kFydeOSCryptoKey =
  fydeos::constants::kFydeOSPolicyVerificationKey;
const size_t kFydeOSCryptoKeyLength =
  fydeos::constants::kFydeOSPolicyVerificationKeyLength;

const char kFydeOSStoreAppId[] = "hidnajblbifdkmheebalalchohohmaef";

}  // namespace fydeos::constants
