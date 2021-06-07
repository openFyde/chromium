// Copyright (c) 2018 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FYDEOS_DBUS_SHELL_STATE_H_
#define FYDEOS_DBUS_SHELL_STATE_H_

#include <stdint.h>

#include <string>

#include "chromeos/chromeos_export.h"

namespace fydeos {

struct CHROMEOS_EXPORT ShellState {
  int32_t code;
  std::string result;

  ShellState();
  std::string ToString() const;
};

}  // namespace fydeos

#endif  // FYDEOS_DBUS_SHELL_STATE_H_
