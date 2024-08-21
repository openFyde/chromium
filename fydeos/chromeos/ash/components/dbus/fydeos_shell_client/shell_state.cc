// Copyright (c) 2018 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fydeos/chromeos/ash/components/dbus/fydeos_shell_client/shell_state.h"

#include "base/format_macros.h"
#include "base/strings/stringprintf.h"

namespace fydeos {

namespace ash {

ShellState::ShellState()
    : code(0){
}

std::string ShellState::ToString() const {
  std::string result_s;
  base::StringAppendF(&result_s,
                      "code = %d ",
                      code);
  base::StringAppendF(&result_s,
                      "result = %s ",
                      result.c_str());
  return result_s;
}

}

}  // namespace fydeos
