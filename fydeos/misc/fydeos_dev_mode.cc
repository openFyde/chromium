// Copyright 2024 The Fyde Innovations Limited. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fydeos/misc/fydeos_dev_mode.h"
#include "base/files/file_util.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/logging.h"
#include "fydeos/chromeos/ash/components/dbus/fydeos_shell_client/fydeos_shell_client.h"

using fydeos::ash::FydeOSShellClient;
using fydeos::ash::ShellState;

namespace fydeos::misc {

namespace {

FydeOSShellClient* GetShellClient() {
  return FydeOSShellClient::Get();
}
const char bin_script[] = "/usr/sbin/crossystem_mode-switch.sh";
const char enable_param[] = "enable-dev_mode";
const char disable_param[] = "disable-dev_mode";
const char query_param[] = "dev_mode";

void OnSetCommandFinished(const bool enabled, base::OnceCallback<void(bool)> callback, std::optional<ShellState> state) {
  if (!state || state->code != 0) {
    LOG(ERROR) << "Set dev mode " << enabled << " failed";
    std::move(callback).Run(false);
    return;
  }
  std::move(callback).Run(true);
}

void OnGetCommandFinished(base::OnceCallback<void(bool)> callback, std::optional<ShellState> state) {
  if (!state || state->code != 0) {
    LOG(ERROR) << "Get dev mode " << " failed";
    std::move(callback).Run(false);
    return;
  }
  auto result = base::TrimWhitespaceASCII(state->result, base::TRIM_ALL);
  const bool enabled = result == "dev";
  std::move(callback).Run(enabled);
}

}  // namespace

bool IsDevModeSwitchSupported() {
  return base::PathExists(base::FilePath(bin_script));
}

void SetDevMode(const bool enabled, base::OnceCallback<void(bool)> callback) {
  FydeOSShellClient* shellClient = GetShellClient();
  if (!shellClient) {
    std::move(callback).Run(false);
    return;
  }
  std::string command = base::StringPrintf("%s %s", bin_script, enabled ? enable_param : disable_param);
  shellClient->SyncExec(command,
      base::BindOnce(&OnSetCommandFinished, enabled, std::move(callback)));
}

void GetDevMode(base::OnceCallback<void(bool)> callback) {
  FydeOSShellClient* shellClient = GetShellClient();
  if (!shellClient) {
    std::move(callback).Run(false);
    return;
  }
  std::string command = bin_script + std::string(" ") + query_param;
  shellClient->SyncExec(command,
      base::BindOnce(&OnGetCommandFinished, std::move(callback)));
}

} // namespace fydeos::misc

