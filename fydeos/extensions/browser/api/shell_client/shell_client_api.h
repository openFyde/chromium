#ifndef FYDEOS_EXTENSIONS_API_SHELL_CLIENT_API_H_
#define FYDEOS_EXTENSIONS_API_SHELL_CLIENT_API_H_

#include <string>
#include "base/memory/weak_ptr.h"
#include "base/scoped_observation.h"
#include "extensions/browser/browser_context_keyed_api_factory.h"
#include "chrome/browser/profiles/profile.h"
#include "extensions/browser/event_router.h"
#include "extensions/browser/extension_function.h"
#include "extensions/browser/extension_function_histogram_value.h"
#include "fydeos/dbus/fydeos_shell_client.h"
#include "fydeos/dbus/shell_state.h"

using ShellState = fydeos::ShellState;
using FydeOSShellClient = fydeos::FydeOSShellClient;

namespace extensions {

class ShellClientEventRouter : public FydeOSShellClient::Observer {
  public:
    explicit ShellClientEventRouter(Profile* profile);
    ~ShellClientEventRouter() override;

    void AddExtensionId(const std::string& extension_id, const std::string& event_name);
    void RemoveExtensionId(const std::string& extension_id, const std::string& event_name);

  private:
    void OnSystemNotificationReceived(int32_t level, const std::string& msg) override;
    void OnCommandNotificationReceived(int32_t handler, int32_t state,
        const std::string& msg) override;
    void OnCustomNotificationReceived(int32_t data, int32_t exdata,
        const std::string& extra) override;
    base::ScopedObservation<FydeOSShellClient, FydeOSShellClient::Observer>
      fydeos_client_observer_;
    Profile* profile_;
    EventRouter* event_router_;
    std::set<std::string> system_extension_ids_;
    std::set<std::string> command_extension_ids_;
    std::set<std::string> custom_extension_ids_;
    base::WeakPtrFactory<ShellClientEventRouter> weak_factory_;
}; //class ShellClientEventRouter

class ShellClientAPI : public BrowserContextKeyedAPI,
                       public EventRouter::Observer {
 public:
  static BrowserContextKeyedAPIFactory<ShellClientAPI>* GetFactoryInstance();
  static ShellClientAPI* Get(content::BrowserContext* context);
  explicit ShellClientAPI(content::BrowserContext* context);
  ~ShellClientAPI() override;
  void Shutdown() override;

    // EventRouter::Observer implementation.
  void OnListenerAdded(const EventListenerInfo& details) override;
  void OnListenerRemoved(const EventListenerInfo& details) override;

 private:
  friend class BrowserContextKeyedAPIFactory<ShellClientAPI>;
    // BrowserContextKeyedAPI implementation.
  static const char* service_name() { return "ShellClientAPI"; }
  static const bool kServiceRedirectedInIncognito = true;
  static const bool kServiceIsNULLWhileTesting = true;

  void RegisterNotifications();

  Profile* profile_;
  std::unique_ptr<ShellClientEventRouter> shell_client_event_router_;

  base::WeakPtrFactory<ShellClientAPI> weak_factory_;

}; //class ShellClientAPI

  class ShellClientExecSyncFunction: public ExtensionFunction {
    ~ShellClientExecSyncFunction() override {}
    ResponseAction Run() override;
    void StateCallback(absl::optional<ShellState> state);
    DECLARE_EXTENSION_FUNCTION("shellClient.execSync", SHELL_SYNC_EXEC)
  };

  class ShellClientExecAsyncFunction: public ExtensionFunction {
    ~ShellClientExecAsyncFunction() override {}
    ResponseAction Run() override;
    void StateCallback(absl::optional<ShellState> state);
    DECLARE_EXTENSION_FUNCTION("shellClient.execAsync", SHELL_ASYNC_EXEC)
  };

  class ShellClientGetTaskStateFunction: public ExtensionFunction {
    ~ShellClientGetTaskStateFunction() override {}
    ResponseAction Run() override;
    void StateCallback(absl::optional<ShellState> state);
    DECLARE_EXTENSION_FUNCTION("shellClient.getTaskState", SHELL_GET_TASK_STATE)
  };

  class ShellClientForceCloseTaskFunction: public ExtensionFunction {
    ~ShellClientForceCloseTaskFunction() override {}
    ResponseAction Run() override;
    void StateCallback(absl::optional<ShellState> state);
    DECLARE_EXTENSION_FUNCTION("shellClient.forceCloseTask", SHELL_FORCE_CLOSE_TASK)
  };

  class ShellClientGetTaskOutputFunction: public ExtensionFunction {
    ~ShellClientGetTaskOutputFunction() override {}
    ResponseAction Run() override;
    void StateCallback(absl::optional<ShellState> state);
    DECLARE_EXTENSION_FUNCTION("shellClient.getTaskOutput", SHELL_GET_TASK_OUTPUT)
  };

  class ShellClientGetDaemonStateFunction: public ExtensionFunction {
    ~ShellClientGetDaemonStateFunction() override {}
    ResponseAction Run() override;
    void StateCallback(absl::optional<ShellState> state);
    DECLARE_EXTENSION_FUNCTION("shellClient.getDaemonState", SHELL_GET_DAEMON_STATE)
  };
} //extensions
#endif // FYDEOS_EXTENSIONS_API_SHELL_CLIENT_API_H_
