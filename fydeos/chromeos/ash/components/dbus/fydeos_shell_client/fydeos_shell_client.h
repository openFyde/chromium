#ifndef FYDEOS_DBUS_SHELL_CLIENT_H_
#define FYDEOS_DBUS_SHELL_CLIENT_H_

#include <stdint.h>

#include <string>
#include <vector>

#include "base/functional/callback.h"
#include "base/observer_list.h"
#include "chromeos/chromeos_export.h"
#include "chromeos/dbus/common/dbus_client.h"
#include "chromeos/dbus/common/dbus_callback.h"
#include "fydeos/chromeos/ash/components/dbus/fydeos_shell_client/shell_state.h"

namespace fydeos {
namespace ash {

class COMPONENT_EXPORT(ASH_DBUS_FYDEOS_SHELL_CLIENT) FydeOSShellClient
    : public chromeos::DBusClient {
  public:
    class Observer {
      public:
       virtual ~Observer() {}
       virtual void OnSystemNotificationReceived(int32_t level,
            const std::string& msg) {}
       virtual void OnCommandNotificationReceived(int32_t handler, int32_t state,
            const std::string& msg) {}
       virtual void OnCustomNotificationReceived(int32_t data, int32_t exdata,
            const std::string& extra) {}
    };
    virtual void AddObserver(Observer* observer) = 0;
    virtual void RemoveObserver(Observer* observer) = 0;
    virtual bool HasObserver(const Observer* observer) const = 0;
    virtual void SyncExec(const std::string& cmd, chromeos::DBusMethodCallback<ShellState> callback) = 0;
    virtual void AsyncExec(const std::string& cmd, chromeos::DBusMethodCallback<ShellState> callback) = 0;
    virtual void GetTaskOutput(int32_t key, int32_t lines, chromeos::DBusMethodCallback<ShellState> callback) = 0;
    virtual void GetTaskState(int32_t key, chromeos::DBusMethodCallback<ShellState> callback) = 0;
    virtual void ForceCloseTask(int32_t key, chromeos::DBusMethodCallback<ShellState> callback) = 0;
    virtual void GetDaemonState(chromeos::DBusMethodCallback<ShellState> callback) = 0;

    // Creates and initializes the global instance. |bus| must not be null.
    static void Initialize(dbus::Bus* bus);

    // Creates and initializes a fake global instance.
    static void InitializeFake();

    // Destroys the global instance if it has been initialized.
    static void Shutdown();

    // Returns the global instance if initialized. May return null.
    static FydeOSShellClient* Get();

  FydeOSShellClient(const FydeOSShellClient&) = delete;
  FydeOSShellClient& operator=(const FydeOSShellClient&) = delete;

  protected:
    // Initialize() should be used instead.
    FydeOSShellClient();
    ~FydeOSShellClient() override;
};

} // namespace ash
} // namespace fydeos
#endif
