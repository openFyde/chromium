#include "fydeos/misc/fydeos_toggle_ota.h"
#include "third_party/abseil-cpp/absl/types/optional.h"
#include "base/logging.h"
#include "base/functional/bind.h"
#include "base/files/file_util.h"
#include "fydeos/chromeos/ash/components/dbus/fydeos_shell_client/fydeos_shell_client.h"

using fydeos::ash::FydeOSShellClient;
using fydeos::ash::ShellState;

namespace fydeos {
namespace misc {

namespace {
  FydeOSShellClient* GetShellClient() {
    return FydeOSShellClient::Get();
  }

  const char kFydeOSOTAIndicatorFile[] = "/mnt/stateful_partition/unencrypted/preserve/.disable_fydeos_ota";

  std::string EnableOTACommand() {
    return std::string("rm ") + kFydeOSOTAIndicatorFile;
  }
  std::string DisableOTACommand() {
    return std::string("touch ") + kFydeOSOTAIndicatorFile;
  }

  void OnSetCommandFinished(const bool enabled, base::OnceCallback<void()> callback, absl::optional<ShellState> state) {
    if (!state || state->code != 0) {
      LOG(ERROR) << "Set FydeOS OTA " << enabled << " failed";
    }
    std::move(callback).Run();
  }
}

void EnableFydeOTA(const bool enabled, base::OnceCallback<void()> callback) {
  FydeOSShellClient* shellClient = GetShellClient();
  if (!shellClient) return;
  const std::string command = enabled ? EnableOTACommand() : DisableOTACommand();
  shellClient->SyncExec(command,
      base::BindOnce(&OnSetCommandFinished, enabled, std::move(callback)));
}

bool GetEnabledFydeOTA() {
  const bool enabled = !base::PathExists(base::FilePath(kFydeOSOTAIndicatorFile));
  return enabled;
}

} // misc
} // fydeos
