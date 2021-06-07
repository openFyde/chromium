// Copyright 2018 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_DBUS_DBUS_CLIENTS_FYDEOS_H_
#define CHROMEOS_DBUS_DBUS_CLIENTS_FYDEOS_H_

#include <memory>

#include "chromeos/chromeos_export.h"
#include "fydeos_shell_client.h"

namespace dbus {
class Bus;
}

namespace chromeos {

// D-Bus clients used only in the browser process.
class CHROMEOS_EXPORT DBusClientsFydeOS {
 public:
  // Creates real implementations if |use_real_clients| is true and fakes
  // otherwise. Fakes are used when running on Linux desktop and in tests.
  explicit DBusClientsFydeOS(bool use_real_clients);
  ~DBusClientsFydeOS();

  void Initialize(dbus::Bus* system_bus);

 private:
  friend class DBusThreadManager;
  friend class DBusThreadManagerSetter;

  std::unique_ptr<fydeos::FydeOSShellClient> fydeos_shell_client_;
};

}  // namespace chromeos

#endif  // CHROMEOS_DBUS_DBUS_CLIENTS_FYDEOS_H_
