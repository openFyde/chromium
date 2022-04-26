// Copyright (c) 2019 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_FYDEOS_SWITCHES_ACCOUNT_CONSTANTS_H_
#define CHROMEOS_FYDEOS_SWITCHES_ACCOUNT_CONSTANTS_H_

#include <string>
#include "chromeos/chromeos_export.h"

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
CHROMEOS_EXPORT extern const char kDefaultFydeOSClassifyURLRequestApiPath[];
extern const size_t kFydeOSSupervisedUserSettingsDefaultSyncIntervalInSeconds;

CHROMEOS_EXPORT extern const char kFydeAPIKeysDevelopersHowToURL[];

}//constants
}//fydeos

#endif
