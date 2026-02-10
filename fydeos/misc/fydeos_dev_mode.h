// Copyright 2024 The Fyde Innovations Limited. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FYDEOS_MISC_FYDEOS_DEV_MODE_H_
#define FYDEOS_MISC_FYDEOS_DEV_MODE_H_

#include "base/functional/callback_forward.h"

namespace fydeos::misc {

bool IsDevModeSwitchSupported();
void SetDevMode(const bool enabled, base::OnceCallback<void(bool)> callback);
void GetDevMode(base::OnceCallback<void(bool)> callback);

}


#endif // !FYDEOS_MISC_FYDEOS_DEV_MODE_H_
