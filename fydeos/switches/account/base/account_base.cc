// Copyright (c) 2022 Fyde Innovations. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fydeos/switches/account/base/account_base.h"

namespace fydeos {
namespace switches {

namespace {
bool is_device_managed = false;
}

void FydeSetDeviceManagedFlag(bool is_managed) {
  is_device_managed = is_managed;
}

bool IsFydeSetDeviceManaged() {
  return is_device_managed;
}

} // namespace switches
} // namespace fydeos
