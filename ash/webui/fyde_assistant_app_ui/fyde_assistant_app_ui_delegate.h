#ifndef ASH_WEBUI_FYDE_ASSISTANT_APP_UI_FYDE_ASSISTANT_UI_DELEGATE_H_
#define ASH_WEBUI_FYDE_ASSISTANT_APP_UI_FYDE_ASSISTANT_UI_DELEGATE_H_

namespace content {
class WebUIDataSource;
}

namespace ash {

class FydeAssistantAppUIDelegate {
 public:
  virtual ~FydeAssistantAppUIDelegate() = default;
  virtual void PopulateLoadTimeData(content::WebUIDataSource* source) = 0;

};

} // namespace ash

#endif // !ASH_WEBUI_FYDE_ASSISTANT_APP_UI_FYDE_ASSISTANT_UI_DELEGATE_H_
