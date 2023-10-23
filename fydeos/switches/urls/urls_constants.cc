// Copyright (c) 2019 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fydeos/switches/urls/urls_constants.h"
#include "fydeos/build/config/buildflags.h"

namespace fydeos::constants {

#if BUILDFLAG(USE_FYDEOS_COM)

const char kDefaultTestUrl[] = "http://store.fydeos.com/204";
const char kFydeOSHomePageUrl[] = "https://fydeos.com";

const char kFydeOSStoreBaseUrl[] = "https://store.fydeos.com";
const char kFydeOSAccountBaseUrl[] = "https://account.fydeos.com";

const char kFydeOSForumURL[] = "https://community.fydeos.com/";
const char kFydeOSRemoteDesktopURL[] = "https://rdp.fydeos.com/";
const char kFydeOSHelpURL[] = "https://fydeos.com/help/";
const char kFydeOSReleaseNotesURL[] = "https://fydeos.com/release";
const char kMultiDeviceLearnMoreURL[] = "https://fydeos.com/docs";
const char kLanguageSettingsLearnMoreUrl[] = "https://fydeos.com/docs/manual/customize-settings/language/manage-your-fydeos-devices-languages/";
const char kLinuxAppsLearnMoreURL[] = "https://fydeos.com/docs/manual/manage-your-apps/add-apps-and-extensions/set-up-linux-on-your-fydeos-device/";
const char kOnlineEulaURLPath[] = "https://fydeos.com/terms-of-service/?agent=oobe";
const char kOnlinePrivacyURLPath[] = "https://fydeos.com/privacy/?agent=oobe";
const char kFydeAccessibilityHelpURL[] = "https://fydeos.com/docs/manual/explore-accessibility";
const char kFydeNewGestureHelpURL[] = "https://fydeos.com/docs/manual/customize-settings/use-tablets/use-gestures-or-buttons-to-navigate-in-tablet-mode";
const char kSmbSharesLearnMoreURL[] = "https://fydeos.com/docs";
const char kCupsPrintLearnMoreURL[] = "https://fydeos.com/docs";
const char kNaturalScrollHelpURL[] = "https://fydeos.com/docs/manual/customize-settings/appearance/use-your-fydeos-device-touchpad/";
const char kFydeOSAccountURL[] = "https://account.fydeos.com";
const char kFydeOSAccountChooserURL[] = "https://account.fydeos.com";
const char kFydeOSPasswordManagerURL[] = "https://account.fydeos.com";

const char kGoogleDriveBuyStorageUrl[] = "https://fydeos.com/docs/knowledge-base";
const char kGoogleDriveOverviewUrl[] = "https://fydeos.com/docs/knowledge-base";
const char kGoogleDriveHelpUrl[] = "https://fydeos.com/docs/knowledge-base";
const char kGoogleDriveOfflineHelpUrl[] = "https://fydeos.com/docs/knowledge-base";
const char kGoogleDriveRootUrl[] = "https://fydeos.com";
const char kHelpURLFormat[] = "https://fydeos.com/docs/knowledge-base/answer/%d";
const char kHelpURLNoTaskForFile[] = "https://fydeos.com/docs/manual/manage-your-apps/files-and-downloads/file-types-and-external-devices-that-work-on-fydeos-device/";
const char kFydeDropUrl[] = "https://drop.fydeos.com";
const char kFydeDiagnosticAppUrl[] = "https://fydeos.com/docs/manual/fix-problems/use-diagnostics-to-troubleshoot";
const char kFydeExperimentTpmFallbackUrl[] = "https://fydeos.com/faq/experimental-tpm-fallback";
const char kEolNotificationURL[] = "https://fydeos.com/content/eol";
const char kKeyboardShortcutHelpPageUrl[] = "https://fydeos.com/docs/knowledge-base/recipes/keyboard-shortcuts";
const char kFydeOSEnableWidevineLearnMoreURL[] = "https://fydeos.com/docs/knowledge-base/recipes/widevine";
const char kWifiHiddenNetworkURL[] = "https://fydeos.com/docs/manual/connect-your-fydeos-device/connect-to-wi-fi-&-other-networks/manage-wifi-networks/";
const char kBluetoothPairingLearnMoreUrl[] = "https://fydeos.com/docs/manual/connect-your-fydeos-device/connect-to-other-devices/connect-to-bluetooth-devices/";
const char kFileManagerHelpURL[] = "https://fydeos.com/docs/manual/manage-your-apps/files-and-downloads/open-save-or-delete-files/";
const char kTabletModeGesturesLearnMoreURL[] = "https://fydeos.com/docs/manual/customize-settings/use-tablets/use-gestures-or-buttons-to-navigate-in-tablet-mode";
const char kRuntimeHostPermissionsHelpURL[] = "https://fydeos.com/docs/manual/manage-your-apps/add-apps-and-extensions/add-apps-and-extensions/";
const char kFingerprintLearnMoreURL[] = "https://fydeos.com/docs/manual/customize-settings/users-and-sync/set-up-and-sign-in-with-fingerprint-on-your-fydeos-device";

const char kFydeOSBackupRestoreLearnMoreURL[] = "https://fydeos.com/docs/manual/customize-settings/fydeos-settings/misc#backup-and-restore";

#else

const char kDefaultTestUrl[] = "http://store.fydeos.io/204";
const char kFydeOSHomePageUrl[] = "https://fydeos.io";

const char kFydeOSStoreBaseUrl[] = "https://store.fydeos.io";
const char kFydeOSAccountBaseUrl[] = "https://account.fydeos.io";

const char kFydeOSForumURL[] = "https://community.fydeos.io/";
const char kFydeOSRemoteDesktopURL[] = "https://rdp.fydeos.io/";
const char kFydeOSHelpURL[] = "https://fydeos.io/help/";
const char kFydeOSReleaseNotesURL[] = "https://fydeos.io/release";
const char kMultiDeviceLearnMoreURL[] = "https://fydeos.io/docs";
const char kLanguageSettingsLearnMoreUrl[] = "https://fydeos.io/docs/manual/customize-settings/language";
const char kLinuxAppsLearnMoreURL[] = "https://fydeos.io/docs/manual/manage-your-apps/add-apps-and-extensions/set-up-linux-on-your-fydeos-device/";
const char kOnlineEulaURLPath[] = "https://fydeos.io/terms-of-service/?agent=oobe";
const char kOnlinePrivacyURLPath[] = "https://fydeos.io/privacy/?agent=oobe";
const char kFydeAccessibilityHelpURL[] = "https://fydeos.io/docs/manual/explore-accessibility";
const char kFydeNewGestureHelpURL[] = "https://fydeos.io/docs/manual/customize-settings/use-tablets/use-gestures-or-buttons-to-navigate-in-tablet-mode";
const char kSmbSharesLearnMoreURL[] = "https://fydeos.io/docs";
const char kCupsPrintLearnMoreURL[] = "https://fydeos.io/docs";
const char kNaturalScrollHelpURL[] = "https://fydeos.io/docs/manual/customize-settings/appearance/use-your-fydeos-device-touchpad/";
const char kFydeOSAccountURL[] = "https://account.fydeos.io";
const char kFydeOSAccountChooserURL[] = "https://account.fydeos.io";
const char kFydeOSPasswordManagerURL[] = "https://account.fydeos.io";

const char kGoogleDriveBuyStorageUrl[] = "https://fydeos.io/docs/knowledge-base";
const char kGoogleDriveOverviewUrl[] = "https://fydeos.io/docs/knowledge-base";
const char kGoogleDriveHelpUrl[] = "https://fydeos.io/docs/knowledge-base";
const char kGoogleDriveOfflineHelpUrl[] = "https://fydeos.io/docs/knowledge-base";
const char kGoogleDriveRootUrl[] = "https://fydeos.io";
const char kHelpURLFormat[] = "https://fydeos.io/docs/knowledge-base/answer/%d";
const char kHelpURLNoTaskForFile[] = "https://fydeos.io/docs/manual/manage-your-apps/files-and-downloads/file-types-and-external-devices-that-work-on-fydeos-device/";
const char kFydeDropUrl[] = "https://drop.fydeos.io";
const char kFydeDiagnosticAppUrl[] = "https://fydeos.io/docs/manual/fix-problems/use-diagnostics-to-troubleshoot";
const char kFydeExperimentTpmFallbackUrl[] = "https://fydeos.io/faq/experimental-tpm-fallback";
const char kEolNotificationURL[] = "https://fydeos.io/content/eol";
const char kKeyboardShortcutHelpPageUrl[] = "https://fydeos.io/docs/knowledge-base/recipes/keyboard-shortcuts";
const char kFydeOSEnableWidevineLearnMoreURL[] = "https://fydeos.io/docs/knowledge-base/recipes/widevine";
const char kWifiHiddenNetworkURL[] = "https://fydeos.io/docs/manual/connect-your-fydeos-device/connect-to-wi-fi-&-other-networks/manage-wifi-networks/";
const char kBluetoothPairingLearnMoreUrl[] = "https://fydeos.io/docs/manual/connect-your-fydeos-device/connect-to-other-devices/connect-to-bluetooth-devices/";
const char kFileManagerHelpURL[] = "https://fydeos.io/docs/manual/manage-your-apps/files-and-downloads/open-save-or-delete-files/";
const char kTabletModeGesturesLearnMoreURL[] = "https://fydeos.io/docs/manual/customize-settings/use-tablets/use-gestures-or-buttons-to-navigate-in-tablet-mode";
const char kRuntimeHostPermissionsHelpURL[] = "https://fydeos.io/docs/manual/manage-your-apps/add-apps-and-extensions/add-apps-and-extensions/";
const char kFingerprintLearnMoreURL[] = "https://fydeos.io/docs/manual/customize-settings/users-and-sync/set-up-and-sign-in-with-fingerprint-on-your-fydeos-device";

const char kFydeOSBackupRestoreLearnMoreURL[] = "https://fydeos.io/docs/manual/customize-settings/fydeos-settings/misc#backup-and-restore";

#endif

}  // fydeos::constants
