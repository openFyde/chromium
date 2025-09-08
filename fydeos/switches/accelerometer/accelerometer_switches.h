// Copyright (c) 2019 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_FYDEOS_SWITCHES_ACCELEROMETER_SWITCHES_H_
#define CHROMEOS_FYDEOS_SWITCHES_ACCELEROMETER_SWITCHES_H_

#include "chromeos/chromeos_export.h"

namespace fydeos {
namespace switches {

CHROMEOS_EXPORT bool IsFydeOSAccelerometer();
CHROMEOS_EXPORT int GetAccelConfig();
CHROMEOS_EXPORT bool IsAccelRevertX();
CHROMEOS_EXPORT bool IsAccelRevertY();
CHROMEOS_EXPORT bool IsAccelRevertZ();
CHROMEOS_EXPORT bool IsAccelRightMove();
CHROMEOS_EXPORT int GetAccelRightMoveBits();
CHROMEOS_EXPORT int GetAccelDataPattern();
CHROMEOS_EXPORT bool IsRotate_90();
CHROMEOS_EXPORT bool IsRotate_180();
CHROMEOS_EXPORT bool IsRotate_270();
CHROMEOS_EXPORT int GetFydeOSAccelerometerReadIntervalInMS();
CHROMEOS_EXPORT bool FydeOSAccelerometerSwapBytes();

} // switches
} // fydeos
#endif
