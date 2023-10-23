// Copyright (c) 2021 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fydeos/prefs/fydeos_pref_names.h"

namespace fydeos {
namespace prefs {


const char kPrefHideFydeOSStoreIcon[] = "hide_fydeos_store_icon";

const char kFydeOSImprovementPlanEnabled[] = "fydeos_improvement_plan_enabled";

// local state
const char kEnableArcIMEGlobally[] = "enable_arc_ime_globally";
const char kCurrentEnableArcIMEGlobally[] = "current_enable_arc_ime_globally";

const char kForceTpmFallbackNecessary[] = "force_tpm_fallback_necessary";
const char kCurrentForceTpmFallback[] = "current_force_tpm_fallback";
const char kForceTpmFallback[] = "force_tpm_fallback";

const char kShowSwitchTabletLaptopButton[] = "show_switch_tablet_laptop_button";
const char kShowRebootButtonInTray[] = "show_reboot_button_in_tray";
const char kShowRotateScreenButton[] = "show_rotate_screen_button";

const char kOfflineAutoSigninAccountIdKey[] = "offline_auto_signin.account_id_key";
const char kOfflineAutoSigninPassword[] = "offline_auto_signin.password";
const char kOfflineAutoSigninPasswordFormat[] = "offline_auto_signin.password_format";
const char kOfflineAutoSigninIsChromeLastSignout[] = "offline_auto_signin.chrome_signout";

const char kFactoryResetRequested[] = "FactoryResetRequested";

const char kRebootRequiredForWidevine[] = "reboot_required_for_widevine";
}
}
