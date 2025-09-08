// Copyright (c) 2019 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_FYDEOS_SWITCHES_SERVICES_SWITCHES_H_
#define CHROMEOS_FYDEOS_SWITCHES_SERVICES_SWITCHES_H_

#include <string>
#include "chromeos/chromeos_export.h"
#include "fydeos/build/config/buildflags.h"

namespace fydeos {
namespace switches {

CHROMEOS_EXPORT bool DisableFydeOSGeolocationAPI();
CHROMEOS_EXPORT bool DisableFydeOSTimezoneAPI();

extern std::string GetFydeOSGeolocationAPIUrl();
extern std::string GetFydeOSTimezoneAPIUrl();

extern std::string GetFydeOSLookingGlassUrl();

extern std::string GetFydeOSAppStoreURL();

extern std::string GetFydeOSWebStoreUpdateUrl();

extern std::string GetFydeOSAssistantWebUrl();

CHROMEOS_EXPORT std::string MayConvertWebStoreUpdateUrl(
    const std::string& url);

#if BUILDFLAG(FYDEOS_DEVICE)
std::string GetFydeOSProductWarrantyUrl();
#endif

} // switches
} // fydeos

#endif
