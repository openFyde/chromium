#ifndef ASH_WEBUI_FYDE_ASSISTANT_APP_UI_FYDE_ASSISTANT_APP_WEBUI_HANDLER_H_
#define ASH_WEBUI_FYDE_ASSISTANT_APP_UI_FYDE_ASSISTANT_APP_WEBUI_HANDLER_H_

#include "content/public/browser/web_ui_message_handler.h"
#include "ash/public/cpp/assistant/controller/assistant_controller.h"
#include "ash/public/cpp/assistant/controller/assistant_controller_observer.h"

namespace ash {

class FydeAssistantWebUIHandler
    : public content::WebUIMessageHandler,
      public AssistantControllerObserver {

 public:
  explicit FydeAssistantWebUIHandler();

  FydeAssistantWebUIHandler(const FydeAssistantWebUIHandler&) = delete;
  FydeAssistantWebUIHandler& operator=(const FydeAssistantWebUIHandler&) =
      delete;

  ~FydeAssistantWebUIHandler() override;

  // content::WebUIMessageHandler:
  void RegisterMessages() override;

  // AssistantControllerObserver:
  void OnDeepLinkReceived(
      assistant::util::DeepLinkType type,
      const std::map<std::string, std::string>& params) override;

 private:
  void OnFydeAssistantSwaInit(const base::Value::List& args);
  void OnRequestCloseAssistant(const base::Value::List& args);

  void OnFydeAssistantOpenUrl(const base::Value::List& args);

  base::ScopedObservation<AssistantController, AssistantControllerObserver>
      assistant_controller_observation_{this};
};

} // namespace ash

#endif // !#ifndef ASH_WEBUI_FYDE_ASSISTANT_APP_UI_FYDE_ASSISTANT_APP_WEBUI_HANDLER_H_
