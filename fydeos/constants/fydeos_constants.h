// Copyright 2024 The Fyde Innovations Limited. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FYDEOS_CONSTANTS_FYDEOS_CONSTANTS_H_
#define FYDEOS_CONSTANTS_FYDEOS_CONSTANTS_H_

#include "fydeos/build/config/buildflags.h"
#include <string>

namespace fydeos::constants {

#if BUILDFLAG(USE_FYDEOS_LICENSE)

enum class LicenseStateType {
  kUnspecified = 0,
  kUnlicensed = 1,
  kLicenseForYouTrial = 2 ,
  kLicenseForYouValid = 3,
  kLicenseForYouExpired = 4,
  kLicenseForEnterpriseTrial = 5,
  kLicenseForEnterpriseValid = 6,
  kLicenseForEnterpriseExpired = 7,
  kMaxValue = kLicenseForEnterpriseExpired,
};

enum class LicenseEnforcementLevel {
  kNone = 0,
  kForcePopup = 1,
  kForceQuit = 2,
  kMaxValue = kForceQuit,
};

#endif

bool ShouldHideExtensionById(const std::string& extension_id);

} // namespace fydeos::constants

#endif // !FYDEOS_CONSTANTS_FYDEOS_CONSTANTS_H_
