// Copyright (c) 2021 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_FYDEOS_PREF_NAMES_H_
#define CHROMEOS_FYDEOS_PREF_NAMES_H_

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

extern const char kShowRebootButtonInTray[];
extern const char kShowRotateScreenButton[];

} // prefs
} // fydeos

#endif
