// Copyright (c) 2019 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fydeos/switches/account/account_switches.h"
#include "fydeos/switches/account/account_constants.h"
#include "base/command_line.h"

namespace fydeos {
namespace switches {

namespace {

const char kFydeOSFamilyLinkApisUrl[] = "fydeos-family-link-apis-url";
const char kFydeOSSupervisedUserSettingsSyncIntervalInSeconds[] = "fydeos-supervised-user-settings-sync-interval";

const char kPolicyManagedByFyde[] = "policy-managed-by-fyde";

const char kFydeFtlServerEndpointSwitch[] = "fyde-ftl-server-endpoint";
const char kFydeRemotingServerEndpointSwitch[] = "fyde-remoting-server-endpoint";
}

bool IsFydeAccountEnabled() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  return command_line->HasSwitch(kFydeAccountEnable) && !command_line->HasSwitch(kFydeAccountForceDisabledForTest);
}

const char kFydeAccountEnable[] = "fyde-account-enabled";
const char kFydeAccountForceDisabledForTest[] = "fyde-account-force-disabled";

bool IsFydeExtendAccountEnabled() {
  return IsFydeAccountEnabled();
}

const char kFydeOSGaiaUrl[] = "fydeos-gaia-url";
const char kFydeOSApisUrl[] = "fydeos-apis-url";
const char kFydeOSDeviceManagementUrl[] = "fydeos-device-management-url";
const char kFydeOSRealtimeReportingUrl[] = "fydeos-realtime-reporting-url";
const char kFydeOSEncryptedReportingUrl[] = "fydeos-encrypted-reporting-url";
const char kFydeOSSyncServiceURL[] = "fydeos-sync-url";

std::string GetFydeOSKidsManagementAPIBaseUrl() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(kFydeOSFamilyLinkApisUrl)) {
    return command_line->GetSwitchValueASCII(kFydeOSFamilyLinkApisUrl);
  } else {
    return std::string(fydeos::constants::kDefaultFydeOSFamilyLinkApisUrl);
  }
}

int GetFydeOSSupervisedUserSettingsSyncInterval() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  int interval = 0;
  if (command_line->HasSwitch(kFydeOSSupervisedUserSettingsSyncIntervalInSeconds)) {
    std::string str = command_line->GetSwitchValueASCII(kFydeOSSupervisedUserSettingsSyncIntervalInSeconds);
    interval = std::stoi(str);
  }
  return interval < 10 ? fydeos::constants::kFydeOSSupervisedUserSettingsDefaultSyncIntervalInSeconds : interval;
}

bool IsPolicyManagedByFyde() {
  return base::CommandLine::ForCurrentProcess()->HasSwitch(kPolicyManagedByFyde) || IsFydeAccountEnabled();
}

bool IsFydeDMServerUrl(const std::string& url) {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(fydeos::switches::kFydeOSDeviceManagementUrl)) {
    return url == command_line->GetSwitchValueASCII(fydeos::switches::kFydeOSDeviceManagementUrl);
  } else {
    return url == fydeos::constants::kDefaultFydeOSDeviceManagementServerUrl;
  }
}


std::string GetFydeFtlServerEndpoint() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(fydeos::switches::kFydeFtlServerEndpointSwitch)) {
    return command_line->GetSwitchValueASCII(fydeos::switches::kFydeFtlServerEndpointSwitch);
  } else {
    return fydeos::constants::kDefaultFydeFtlServerEndpoint;
  }
}

std::string GetFydeRemotingServerEndpoint() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(fydeos::switches::kFydeRemotingServerEndpointSwitch)) {
    return command_line->GetSwitchValueASCII(fydeos::switches::kFydeRemotingServerEndpointSwitch);
  } else {
    return fydeos::constants::kDefaultFydeRemotingServerEndpoint;
  }
}


}// namespace switches
}// namespace fydeos
