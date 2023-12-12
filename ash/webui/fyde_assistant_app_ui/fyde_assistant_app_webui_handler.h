#ifndef ASH_WEBUI_FYDE_ASSISTANT_APP_UI_FYDE_ASSISTANT_APP_WEBUI_HANDLER_H_
#define ASH_WEBUI_FYDE_ASSISTANT_APP_UI_FYDE_ASSISTANT_APP_WEBUI_HANDLER_H_

#include "content/public/browser/web_ui_message_handler.h"
#include "ash/webui/fyde_assistant_app_ui/fyde_assistant_app_ui.h"
#include "ash/public/cpp/assistant/controller/assistant_controller.h"
#include "ash/public/cpp/assistant/controller/assistant_controller_observer.h"
#include "ash/assistant/model/assistant_ui_model_observer.h"
#include "ui/native_theme/native_theme.h"
#include "ui/native_theme/native_theme_observer.h"
#include "ash/fydeos_ai/fydeos_ai_view.h"

namespace ash {

class FydeAssistantWebUIHandler
    : public content::WebUIMessageHandler,
      public ui::NativeThemeObserver,
      public AssistantUiModelObserver,
      public FydeAssistantViewObserver,
      public AssistantControllerObserver {

 public:
  explicit FydeAssistantWebUIHandler(FydeAssistantAppUI* fydeAsisstantAppUi);

  FydeAssistantWebUIHandler(const FydeAssistantWebUIHandler&) = delete;
  FydeAssistantWebUIHandler& operator=(const FydeAssistantWebUIHandler&) =
      delete;

  ~FydeAssistantWebUIHandler() override;

  // content::WebUIMessageHandler:
  void RegisterMessages() override;

  // ash::AssistantUiModelObserver:
  void OnUiVisibilityChanged(
      AssistantVisibility new_visibility,
      AssistantVisibility old_visibility,
      absl::optional<AssistantEntryPoint> entry_point,
      absl::optional<AssistantExitPoint> exit_point) override;

  // AssistantControllerObserver:
  void OnDeepLinkReceived(
      assistant::util::DeepLinkType type,
      const std::map<std::string, std::string>& params) override;

  // ui::NativeThemeObserver:
  void OnNativeThemeUpdated(ui::NativeTheme* observed_theme) override;

  void OnBubbleQueryChanged(const FydeAssistantViewObserver::ClipboardItemForAssistant& item) override;
  void OnBubbleVisibilityChanged(bool visible) override;
 private:
  void OnFydeAssistantSwaInit(const base::Value::List& args);
  void OnRequestCloseAssistant(const base::Value::List& args);

  void OnFydeAssistantOpenUrl(const base::Value::List& args);
  void HandleSetAssistantBubbleRect(const base::Value::List& args);

  raw_ptr<FydeAssistantAppUI, ExperimentalAsh> fyde_assistant_app_ui_;

  base::ScopedObservation<AssistantController, AssistantControllerObserver>
      assistant_controller_observation_{this};

  base::ScopedObservation<ui::NativeTheme, ui::NativeThemeObserver>
      theme_observation_{this};
};

} // namespace ash

#endif // !#ifndef ASH_WEBUI_FYDE_ASSISTANT_APP_UI_FYDE_ASSISTANT_APP_WEBUI_HANDLER_H_
