#ifndef CHROME_BROWSER_ASH_WEB_APPLICATIONS_FYDE_ASSISTANT_APP_UI_DELEGATE_H_
#define CHROME_BROWSER_ASH_WEB_APPLICATIONS_FYDE_ASSISTANT_APP_UI_DELEGATE_H_

#include "ash/webui/fyde_assistant_app_ui/fyde_assistant_app_ui_delegate.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"

namespace content {
class WebUI;
}

class ChromeFydeAssistantAppUIDelegate : public ash::FydeAssistantAppUIDelegate {
 public:
  explicit ChromeFydeAssistantAppUIDelegate(content::WebUI* web_ui);

  ChromeFydeAssistantAppUIDelegate(const ChromeFydeAssistantAppUIDelegate&) = delete;
  ChromeFydeAssistantAppUIDelegate& operator=(const ChromeFydeAssistantAppUIDelegate&) = delete;
  ~ChromeFydeAssistantAppUIDelegate() override;

  // FydeAssistantAppUIDelegate:
  void PopulateLoadTimeData(content::WebUIDataSource* source) override;

  ash::FydeAssistantAppUIDelegate::ColorInfo GetSystemColorInfo() override;

 private:
  raw_ptr<content::WebUI, ExperimentalAsh> web_ui_;  // Owns |this|.
  base::WeakPtrFactory<ChromeFydeAssistantAppUIDelegate> weak_ptr_factory_{this};
};

#endif // !#ifndef CHROME_BROWSER_ASH_WEB_APPLICATIONS_FYDE_ASSISTANT_APP_UI_DELEGATE_H_
