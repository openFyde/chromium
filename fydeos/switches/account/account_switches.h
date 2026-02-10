// Copyright (c) 2019 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_FYDEOS_SWITCHES_ACCOUNT_SWITCHES_H_
#define CHROMEOS_FYDEOS_SWITCHES_ACCOUNT_SWITCHES_H_

#include <string>
#include "chromeos/chromeos_export.h"

namespace fydeos {
namespace switches {

CHROMEOS_EXPORT bool IsFydeAccountEnabled();
CHROMEOS_EXPORT bool IsFydeExtendAccountEnabled();

CHROMEOS_EXPORT extern const char kFydeAccountEnable[];
CHROMEOS_EXPORT extern const char kFydeAccountForceDisabledForTest[];

CHROMEOS_EXPORT extern const char kFydeOSGaiaUrl[];
CHROMEOS_EXPORT extern const char kFydeOSApisUrl[];
CHROMEOS_EXPORT extern const char kFydeOSDeviceManagementUrl[];
CHROMEOS_EXPORT extern const char kFydeOSRealtimeReportingUrl[];
CHROMEOS_EXPORT extern const char kFydeOSEncryptedReportingUrl[];
CHROMEOS_EXPORT extern const char kFydeOSSyncServiceURL[];
extern std::string GetFydeOSKidsManagementAPIBaseUrl();
CHROMEOS_EXPORT int GetFydeOSSupervisedUserSettingsSyncInterval();
CHROMEOS_EXPORT bool IsPolicyManagedByFyde();
CHROMEOS_EXPORT bool IsBasicLicenseDevice();
CHROMEOS_EXPORT std::string GetFydeBasicLicenseToken();
CHROMEOS_EXPORT std::string GetFydeAccountUrl();

bool IsFydeDMServerUrl(const std::string& url);

CHROMEOS_EXPORT std::string GetFydeFtlServerEndpoint();
CHROMEOS_EXPORT std::string GetFydeRemotingServerEndpoint();

}
}

#endif
