// Copyright (c) 2019 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fydeos/switches/account/account_constants.h"
#include "fydeos/build/config/buildflags.h"

namespace fydeos::constants {

#if BUILDFLAG(USE_FYDEOS_COM)

const char kDefaultFydeOSGaiaUrl[] = "https://account.fydeos.com";
const char kDefaultFydeOSApisBaseUrl[] = "https://apis.fydeos.com";
const char kDefaultFydeOSDeviceManagementServerUrl[] =
  "https://policy.fydeos.com";
const char kDefaultFydeOSRealtimeReportingServerUrl[] =
  "https://apis.fydeos.com/report/events";
const char kDefaultFydeOSEncryptedReportingServerUrl[] =
  "https://apis.fydeos.com/report/record";
const char kFydeOSSyncDevServerUrl[] =
  "https://clients4.fydeos.com/chrome-sync/dev";
const char kFydeOSSyncServerUrl[] = "https://clients4.fydeos.com/chrome-sync";
const char kDefaultFydeOSFamilyLinkApisUrl[] =
  "https://familylink-apis.fydeos.com/kidsmanagement/v1/";

const char kDefaultFydeFtlServerEndpoint[] = "im.fydeos.com";
const char kDefaultFydeRemotingServerEndpoint[] = "remoting.fydeos.com";

#else

const char kDefaultFydeOSGaiaUrl[] = "https://account.fydeos.io";
const char kDefaultFydeOSApisBaseUrl[] = "https://apis.fydeos.io";
const char kDefaultFydeOSDeviceManagementServerUrl[] =
  "https://policy.fydeos.io";
const char kDefaultFydeOSRealtimeReportingServerUrl[] =
  "https://apis.fydeos.io/report/events";
const char kDefaultFydeOSEncryptedReportingServerUrl[] =
  "https://apis.fydeos.io/report/record";
const char kFydeOSSyncDevServerUrl[] =
  "https://clients4.fydeos.io/chrome-sync/dev";
const char kFydeOSSyncServerUrl[] = "https://clients4.fydeos.io/chrome-sync";
const char kDefaultFydeOSFamilyLinkApisUrl[] =
  "https://familylink-apis.fydeos.io/kidsmanagement/v1/";

const char kDefaultFydeFtlServerEndpoint[] = "im.fydeos.io";
const char kDefaultFydeRemotingServerEndpoint[] = "remoting.fydeos.io";

#endif

const size_t kFydeOSSupervisedUserSettingsDefaultSyncIntervalInSeconds = 600;
const char kFydeEnrollmentTokenFilePath[] =
  "/usr/share/oem/fydeos_enroll_token";

#if BUILDFLAG(IS_OPENFYDE)
const char kFydeAPIKeysDevelopersHowToURL[] =
  "https://openfyde.com/docs/developers/how-to-get-api-keys";
#endif

}  // namespace fydeos::constants
