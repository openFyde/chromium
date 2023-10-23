#ifndef ASH_WEBUI_FYDE_ASSISTANT_APP_UI_FYDE_ASSISTANT_APP_UI_H_
#define ASH_WEBUI_FYDE_ASSISTANT_APP_UI_FYDE_ASSISTANT_APP_UI_H_

#include "content/public/browser/web_ui.h"
#include "content/public/browser/web_ui_controller.h"
#include "ash/webui/fyde_assistant_app_ui/fyde_assistant_app_ui_delegate.h"

namespace ash {

class FydeAssistantAppUI : public content::WebUIController {
 public:
  explicit FydeAssistantAppUI(content::WebUI* web_ui,
                              std::unique_ptr<FydeAssistantAppUIDelegate> delegate);
  FydeAssistantAppUI(const FydeAssistantAppUI&) = delete;
  FydeAssistantAppUI& operator=(const FydeAssistantAppUI&) = delete;
  ~FydeAssistantAppUI() override;

  FydeAssistantAppUIDelegate* delegate() { return delegate_.get(); }

private:
  std::unique_ptr<FydeAssistantAppUIDelegate> delegate_;
  WEB_UI_CONTROLLER_TYPE_DECL();
};

} // namespace ash


#endif  // FYDEOS_ASH_WEBUI_FYDE_ASSISTANT_APP_UI_FYDE_ASSISTANT_APP_UI_H_
