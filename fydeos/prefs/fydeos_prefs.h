// Copyright (c) 2021 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_FYDEOS_PREFS_H_
#define CHROMEOS_FYDEOS_PREFS_H_

#include "fydeos/prefs/fydeos_pref_names.h"

class PrefRegistrySimple;
class PrefService;

namespace fydeos {

namespace prefs {

void RegisterLocalStatePrefs(PrefRegistrySimple* registry);
void RegisterProfilePrefs(PrefRegistrySimple* registry);

void KeepCurrentPrefs(PrefService* local_state);
void SetNotNecessaryForceTpmFallback(PrefService* local_state);

void ClearRebootMarkPrefs(PrefService* local_state);
void ClearOneShotProfilePrefs(PrefService* prefs);

} // prefs

} // fydeos
#endif /* ifndef FYDEOS_PREFS_H */
