// Copyright (c) 2019 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_FYDEOS_SWITCHES_URLS_CONSTANTS_
#define CHROMEOS_FYDEOS_SWITCHES_URLS_CONSTANTS_

#include <string>
#include "chromeos/chromeos_export.h"
#include "fydeos/build/config/buildflags.h"

namespace fydeos::constants {

CHROMEOS_EXPORT extern const char kDefaultTestUrl[];
CHROMEOS_EXPORT extern const char kFydeOSHomePageUrl[];
CHROMEOS_EXPORT extern const char kOpenFydeHomePageUrl[];

CHROMEOS_EXPORT extern const char kFydeOSStoreBaseUrl[];
CHROMEOS_EXPORT extern const char kFydeOSAccountBaseUrl[];

extern const char kFydeOSForumURL[];
extern const char kFydeOSRemoteDesktopURL[];
extern const char kFydeOSNotesAppURL[];
extern const char kFydeOSHelpURL[];
extern const char kFydeOSReleaseNotesURL[];
extern const char kFydeOSNewsURL[];
extern const char kMultiDeviceLearnMoreURL[];
extern const char kLanguageSettingsLearnMoreUrl[];
extern const char kLinuxAppsLearnMoreURL[];
extern const char kOnlineEulaURLPath[];
extern const char kOnlinePrivacyURLPath[];
extern const char kEulaURLPath[];
extern const char kPrivacyURLPath[];
extern const char kFydeAccessibilityHelpURL[];
extern const char kFydeNewGestureHelpURL[];
extern const char kSmbSharesLearnMoreURL[];
extern const char kCupsPrintLearnMoreURL[];
extern const char kNaturalScrollHelpURL[];
extern const char kControlledScrollingHelpURL[];
extern const char kTimeZoneSettingsLearnMoreURL[];
extern const char kResetProfileSettingsLearnMoreURL[];
extern const char kCrosBatterySaverLearnMoreURL[];

CHROMEOS_EXPORT extern const char kFydeOSAccountURL[];
CHROMEOS_EXPORT extern const char kFydeOSAccountChooserURL[];
CHROMEOS_EXPORT extern const char kFydeOSPasswordManagerURL[];

CHROMEOS_EXPORT extern const char kGoogleDriveBuyStorageUrl[];
CHROMEOS_EXPORT extern const char kGoogleDriveOverviewUrl[];
CHROMEOS_EXPORT extern const char kGoogleDriveHelpUrl[];
CHROMEOS_EXPORT extern const char kGoogleDriveOfflineHelpUrl[];
CHROMEOS_EXPORT extern const char kGoogleDriveRootUrl[];
CHROMEOS_EXPORT extern const char kHelpURLFormat[];
CHROMEOS_EXPORT extern const char kHelpURLNoTaskForFile[];
CHROMEOS_EXPORT extern const char kFydeDropUrl[];

CHROMEOS_EXPORT extern const char kFydeExperimentTpmFallbackUrl[];

extern const char kEolNotificationURL[];
extern const char kKeyboardShortcutHelpPageUrl[];
extern const char kFydeOSEnableWidevineLearnMoreURL[];

extern const char kWifiHiddenNetworkURL[];
extern const char kBluetoothPairingLearnMoreUrl[];
extern const char kFileManagerHelpURL[];
extern const char kTabletModeGesturesLearnMoreURL[];
extern const char kRuntimeHostPermissionsHelpURL[];
extern const char kFingerprintLearnMoreURL[];

extern const char kFydeOSBackupRestoreLearnMoreURL[];

extern const char kFydeOSToggleArcMediaAutoScanLearnMoreURL[];

extern const char kFydeOSDevModeTransitionLearnMoreURL[];

#if BUILDFLAG(FYDEOS_DEVICE)
extern const char kFydeOSProductWarrantyDefaultURL[];
#endif

#if !BUILDFLAG(USE_FYDEOS_COM)
extern const char kFydeOSDiscordServerURL[];
extern const char kFydeOSTelegramGroupURL[];
#endif
}  // namespace fydeos::constants

#endif
