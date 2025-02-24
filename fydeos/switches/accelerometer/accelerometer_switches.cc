// Copyright (c) 2019 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fydeos/switches/accelerometer/accelerometer_switches.h"
#include "base/command_line.h"

namespace fydeos {
namespace switches {

namespace {

const char kAccelConfigIndex[] = "fydeos-accel-config";
const char kAccelRevertX[] = "fydeos-accel-revert-x";
const char kAccelRevertY[] = "fydeos-accel-revert-y";
const char kAccelRevertZ[] = "fydeos-accel-revert-z";
const char kAccelRightMoveBits[] = "fydeos-accel-right-move";
const char kAccelDataPattern[] = "fydeos-accel-pattern";
const char kRotate_90[] = "fydeos-rotate-90";
const char kRotate_180[] = "fydeos-rotate-180";
const char kRotate_270[] = "fydeos-rotate-270";
const char kFydeOSAccelerometerReadInterval[] = "fydeos-accel-read-interval";
const char kFydeOSAccelerometerSwapBytes[] = "fydeos-accel-swap-bytes";

}

bool IsFydeOSAccelerometer() {
  return base::CommandLine::ForCurrentProcess()->HasSwitch(kAccelConfigIndex);
}

int GetAccelConfig() {
  std::string indexStr = base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(kAccelConfigIndex);
  if (indexStr.empty())
    return 0;
  return std::stoi(indexStr);
}

bool IsAccelRevertX() {
  return base::CommandLine::ForCurrentProcess()->HasSwitch(kAccelRevertX);
}

bool IsAccelRevertY() {
  return base::CommandLine::ForCurrentProcess()->HasSwitch(kAccelRevertY);
}

bool IsAccelRevertZ() {
  return base::CommandLine::ForCurrentProcess()->HasSwitch(kAccelRevertZ);
}

bool IsAccelRightMove() {
  return base::CommandLine::ForCurrentProcess()->HasSwitch(kAccelRightMoveBits);
}

int GetAccelRightMoveBits() {
  int ret = 0;
  if (IsAccelRightMove()) {
    ret = 4;
    std::string rightBitsStr = base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(kAccelRightMoveBits);
    if (!rightBitsStr.empty())
      ret = std::stoi(rightBitsStr);
  }
  return ret;
}

int GetAccelDataPattern() {
  std::string patternStr = base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(kAccelDataPattern);
  if (patternStr.empty())
    return 0;
  return std::stoi(patternStr);
}

bool IsRotate_90(){
  return base::CommandLine::ForCurrentProcess()->HasSwitch(kRotate_90);
}

bool IsRotate_180(){
  return base::CommandLine::ForCurrentProcess()->HasSwitch(kRotate_180);
}

bool IsRotate_270(){
  return base::CommandLine::ForCurrentProcess()->HasSwitch(kRotate_270);
}

int GetFydeOSAccelerometerReadIntervalInMS() {
  std::string patternStr = base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(kFydeOSAccelerometerReadInterval);
  if (patternStr.empty())
    return 0;
  return std::stoi(patternStr);
}

bool FydeOSAccelerometerSwapBytes() {
  return base::CommandLine::ForCurrentProcess()->HasSwitch(kFydeOSAccelerometerSwapBytes);
}

} // switches
} // fydeos
