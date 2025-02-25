// Copyright (c) 2019 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_FYDEOS_SWITCHES_SERVICES_CONSTANTS_H_
#define CHROMEOS_FYDEOS_SWITCHES_SERVICES_CONSTANTS_H_

#include "chromeos/chromeos_export.h"
#include <string>

namespace fydeos {
namespace constants {

CHROMEOS_EXPORT extern const char kDefaultFydeOSGeolocationAPIUrl[];
CHROMEOS_EXPORT extern const char kDefaultFydeOSTimezoneAPIUrl[];

CHROMEOS_EXPORT extern const char kDefaultFydeOSLookingGlassUrl[];

CHROMEOS_EXPORT extern const char kFydeOSFeedbackPostUrl[];

CHROMEOS_EXPORT extern const uint8_t* kFydeOSCryptoKey;
extern const size_t kFydeOSCryptoKeyLength;

extern const char kFydeOSStoreAppId[];

CHROMEOS_EXPORT extern const char kFydeOSWebStoreUpdateURL[];

CHROMEOS_EXPORT extern const char kFydeOSAssistantDefaultWebUrl[];
} // constants
} // fydeos

#endif
