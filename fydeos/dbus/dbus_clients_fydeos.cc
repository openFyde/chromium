// Copyright 2018 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fydeos/dbus/dbus_clients_fydeos.h"

#include "base/command_line.h"
#include "base/logging.h"
#include "fydeos/dbus/fydeos_shell_client.h"
#include "chromeos/dbus/dbus_thread_manager.h"

namespace chromeos {
DBusClientsFydeOS::DBusClientsFydeOS(bool  use_real_clients) {
  if (use_real_clients) {
    fydeos_shell_client_.reset(fydeos::FydeOSShellClient::Create());
    VLOG(1) << "Create new fydeos_shell_client";
  }else{
    fydeos_shell_client_.reset(nullptr);
    VLOG(1) << "Empty fydeos_shell_client";
  }
}

DBusClientsFydeOS::~DBusClientsFydeOS() = default;

void DBusClientsFydeOS::Initialize(dbus::Bus* system_bus) {
  if (fydeos_shell_client_)
    fydeos_shell_client_->Init(system_bus);
}
}// end of chromeos
