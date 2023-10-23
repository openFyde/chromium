// Copyright (c) 2019 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_FYDEOS_SWITCHES_SERVICES_SWITCHES_H_
#define CHROMEOS_FYDEOS_SWITCHES_SERVICES_SWITCHES_H_

#include <string>
#include "chromeos/chromeos_export.h"

namespace fydeos {
namespace switches {

CHROMEOS_EXPORT bool DisableFydeOSGeolocationAPI();
CHROMEOS_EXPORT bool DisableFydeOSTimezoneAPI();

extern std::string GetFydeOSGeolocationAPIUrl();
extern std::string GetFydeOSTimezoneAPIUrl();

extern std::string GetFydeOSLookingGlassUrl();

extern std::string GetFydeOSAppStoreURL();

extern std::string GetFydeOSWebStoreUpdateUrl();

CHROMEOS_EXPORT std::string MayConvertWebStoreUpdateUrl(
    const std::string& url);

} // switches
} // fydeos

#endif
