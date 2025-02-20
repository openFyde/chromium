#include "chrome/browser/ash/system_web_apps/apps/fyde_assistant_app_ui_delegate.h"
#include "content/public/browser/web_ui.h"
#include "content/public/browser/web_ui_data_source.h"
#include "chrome/browser/profiles/profile.h"

ChromeFydeAssistantAppUIDelegate::ChromeFydeAssistantAppUIDelegate(content::WebUI* web_ui)
    : web_ui_(web_ui) {}

ChromeFydeAssistantAppUIDelegate::~ChromeFydeAssistantAppUIDelegate() = default;

void ChromeFydeAssistantAppUIDelegate::PopulateLoadTimeData(content::WebUIDataSource* source) {
  Profile* profile = Profile::FromWebUI(web_ui_);
  source->AddString("user", profile->GetProfileUserName());
}
