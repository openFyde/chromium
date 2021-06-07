#ifndef FYDEOS_DBUS_SHELL_CLIENT_H_
#define FYDEOS_DBUS_SHELL_CLIENT_H_

#include <stdint.h>

#include <string>
#include <vector>

#include "base/callback.h"
#include "base/observer_list.h"
#include "chromeos/chromeos_export.h"
#include "chromeos/dbus/common/dbus_client.h"
#include "chromeos/dbus/common/dbus_method_call_status.h"
#include "fydeos/dbus/shell_state.h"
namespace chromeos {
  class DBusClientsFydeOS;
}

namespace fydeos {
class CHROMEOS_EXPORT FydeOSShellClient: public chromeos::DBusClient {
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
    ~FydeOSShellClient() override;
    virtual void AddObserver(Observer* observer) = 0;
    virtual void RemoveObserver(Observer* observer) = 0;
    virtual bool HasObserver(const Observer* observer) const = 0;
    virtual void SyncExec(const std::string& cmd, chromeos::DBusMethodCallback<ShellState> callback) = 0;
    virtual void AsyncExec(const std::string& cmd, chromeos::DBusMethodCallback<ShellState> callback) = 0;
    virtual void GetTaskOutput(int32_t key, int32_t lines, chromeos::DBusMethodCallback<ShellState> callback) = 0;
    virtual void GetTaskState(int32_t key, chromeos::DBusMethodCallback<ShellState> callback) = 0;
    virtual void ForceCloseTask(int32_t key, chromeos::DBusMethodCallback<ShellState> callback) = 0;
    virtual void GetDaemonState(chromeos::DBusMethodCallback<ShellState> callback) = 0;
    static FydeOSShellClient* Create();
  protected:
    FydeOSShellClient();
  private:
    friend class chromeos::DBusClientsFydeOS;
};
} //namespace chromeos
#endif
