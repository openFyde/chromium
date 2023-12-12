#ifndef CHROME_BROWSER_ASH_WEB_APPLICATIONS_FYDE_ASSISTANT_APP_INFO_H_
#define CHROME_BROWSER_ASH_WEB_APPLICATIONS_FYDE_ASSISTANT_APP_INFO_H_


#include "chrome/browser/ash/system_web_apps/types/system_web_app_delegate.h"

struct WebAppInstallInfo;

class FydeAssistantAppDelegate : public ash::SystemWebAppDelegate {
  public:
    explicit FydeAssistantAppDelegate(Profile* profile);

  // ash::SystemWebAppDelegate overrides:
  std::unique_ptr<WebAppInstallInfo> GetWebAppInfo() const override;
  bool ShouldCaptureNavigations() const override;
  bool IsAppEnabled() const override;
  // NOTE: there are more virtual methods in the base class.
};

// Return a WebAppInstallInfo used to install the app.
std::unique_ptr<WebAppInstallInfo> CreateWebAppInfoForFydeAssistantApp();

#endif  // FYDEOS_CHROME_BROWSER_ASH_WEB_APPLICATIONS_START_ARC_SETTINGS_WEB_APP_INFO_H_
