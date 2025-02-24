// Copyright (c) 2021 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fydeos/prefs/fydeos_prefs.h"
#include "base/logging.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/pref_registry_simple.h"

namespace fydeos {
namespace prefs {

void RegisterProfilePrefs(PrefRegistrySimple* registry) {
  registry->RegisterBooleanPref(kFydeOSImprovementPlanEnabled, false);
}

void RegisterLocalStatePrefs(PrefRegistrySimple* registry) {
  registry->RegisterBooleanPref(kForceTpmFallbackNecessary, true);
  registry->RegisterBooleanPref(kCurrentForceTpmFallback, false);
  registry->RegisterBooleanPref(kForceTpmFallback, false);

  registry->RegisterBooleanPref(kEnableArcIMEGlobally, false);
  registry->RegisterBooleanPref(kCurrentEnableArcIMEGlobally, false);

  registry->RegisterStringPref(kOfflineAutoSigninAccountIdKey, std::string());
  registry->RegisterStringPref(kOfflineAutoSigninPassword, std::string());
  registry->RegisterStringPref(kOfflineAutoSigninPasswordFormat, std::string());
  registry->RegisterBooleanPref(kOfflineAutoSigninIsChromeLastSignout, false);

  registry->RegisterBooleanPref(kRebootRequiredForWidevine, false);
}

void KeepCurrentPrefs(PrefService* local_state) {
  // prefix with `kCurrent` prefs set here, and read only in other situation
  bool tpm_fallback = local_state->GetBoolean(fydeos::prefs::kForceTpmFallback);
  local_state->SetBoolean(kCurrentForceTpmFallback, tpm_fallback);

  bool enable_arc_ime_globally = local_state->GetBoolean(fydeos::prefs::kEnableArcIMEGlobally);
  local_state->SetBoolean(kCurrentEnableArcIMEGlobally, enable_arc_ime_globally);
}

void SetNotNecessaryForceTpmFallback(PrefService* local_state) {
  local_state->SetBoolean(kForceTpmFallbackNecessary, false);
  local_state->SetBoolean(kCurrentForceTpmFallback, false);
  local_state->SetBoolean(kForceTpmFallback, false);
}

void ClearRebootMarkPrefs(PrefService* local_state) {
  local_state->SetBoolean(kRebootRequiredForWidevine, false);
}

} // prefs
} // fydeos
