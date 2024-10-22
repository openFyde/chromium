// Copyright (c) 2021 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_FYDEOS_PREF_NAMES_H_
#define CHROMEOS_FYDEOS_PREF_NAMES_H_

#include "fydeos/build/config/buildflags.h"

namespace fydeos {
namespace prefs {


extern const char kPrefHideFydeOSStoreIcon[]; // registered in chrome/browser/ui/browser_ui_prefs.cc

extern const char kFydeOSImprovementPlanEnabled[];

// local state
extern const char kCurrentEnableArcIMEGlobally[];
extern const char kEnableArcIMEGlobally[];

extern const char kForceTpmFallbackNecessary[];
extern const char kCurrentForceTpmFallback[];
extern const char kForceTpmFallback[];

extern const char kShowSwitchTabletLaptopButton[];
extern const char kShowRebootButtonInTray[];
extern const char kShowRotateScreenButton[];

extern const char kOfflineAutoSigninAccountIdKey[];
extern const char kOfflineAutoSigninPassword[];
extern const char kOfflineAutoSigninPasswordFormat[];
extern const char kOfflineAutoSigninIsChromeLastSignout[];

// same with kFactoryResetRequested in chrome/common/pref_names.cc
// dep conflict issue
extern const char kFactoryResetRequested[];

extern const char kRebootRequiredForWidevine[];

extern const char kFydeAssistantEnabled[];
extern const char kFydeAssistantExtraAcceleratorEnabled[];

extern const char kFydeOSArcMediaAutoScanEnabled[];

#if BUILDFLAG(USE_FYDEOS_LICENSE)
extern const char kFydeLicenseShouldShowInSettings[];
extern const char kFydeLicenseStateType[];
extern const char kFydeLicenseEnforcementLevel[];
extern const char kFydeLicenseEnforcementLogOutInterval[];
#endif


#if BUILDFLAG(USE_FYDEOS_COM)
extern const char kCrostiniInstallerNotificationUserInteracted[];
#endif

} // prefs
} // fydeos

#endif
