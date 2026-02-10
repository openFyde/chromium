// Copyright (c) 2022 Fyde Innovations. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FYDEOS_SWITCHES_ACCOUNT_ACCOUNT_BASE_H_
#define FYDEOS_SWITCHES_ACCOUNT_ACCOUNT_BASE_H_

#include "chromeos/chromeos_export.h"

namespace fydeos {
namespace switches {

CHROMEOS_EXPORT void FydeSetDeviceManagedFlag(bool is_managed);
CHROMEOS_EXPORT bool IsFydeSetDeviceManaged();

} // namespace switches
} // namespace fydeos


#endif  // FYDEOS_SWITCHES_ACCOUNT_ACCOUNT_BASE_H_
