// Copyright (c) 2019 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_FYDEOS_SWITCHES_ACCOUNT_CONSTANTS_H_
#define CHROMEOS_FYDEOS_SWITCHES_ACCOUNT_CONSTANTS_H_

#include <string>
#include "chromeos/chromeos_export.h"
#include "fydeos/build/config/buildflags.h"

namespace fydeos {
namespace constants {

CHROMEOS_EXPORT extern const char kDefaultFydeOSGaiaUrl[];
CHROMEOS_EXPORT extern const char kDefaultFydeOSApisBaseUrl[];
CHROMEOS_EXPORT extern const char kDefaultFydeOSDeviceManagementServerUrl[];
CHROMEOS_EXPORT extern const char kDefaultFydeOSRealtimeReportingServerUrl[];
CHROMEOS_EXPORT extern const char kDefaultFydeOSEncryptedReportingServerUrl[];
CHROMEOS_EXPORT extern const char kFydeOSSyncDevServerUrl[];
CHROMEOS_EXPORT extern const char kFydeOSSyncServerUrl[];

CHROMEOS_EXPORT extern const char kDefaultFydeOSFamilyLinkApisUrl[];

CHROMEOS_EXPORT extern const char kDefaultFydeFtlServerEndpoint[];
CHROMEOS_EXPORT extern const char kDefaultFydeRemotingServerEndpoint[];

extern const size_t kFydeOSSupervisedUserSettingsDefaultSyncIntervalInSeconds;

#if BUILDFLAG(IS_OPENFYDE)
CHROMEOS_EXPORT extern const char kFydeAPIKeysDevelopersHowToURL[];
#endif

CHROMEOS_EXPORT extern const char kFydeEnrollmentTokenFilePath[];

CHROMEOS_EXPORT extern const char kFydeBlockEnrollFydePath[];
CHROMEOS_EXPORT extern const char kFydeBlockEnrollGooglePath[];
}//constants
}//fydeos

#endif
