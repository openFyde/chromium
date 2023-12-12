#include "chrome/browser/ash/web_applications/fyde_assistant_app_info.h"

#include "ash/constants/ash_features.h"
#include "ash/webui/fyde_assistant_app_ui/url_constants.h"
#include "ash/webui/grit/ash_fyde_assistant_app_resources.h"
#include "chrome/browser/ash/web_applications/system_web_app_install_utils.h"

#include <memory>

std::unique_ptr<WebAppInstallInfo> CreateWebAppInfoForFydeAssistantApp() {
  std::unique_ptr<WebAppInstallInfo> info =
      std::make_unique<WebAppInstallInfo>();
  info->start_url = GURL(ash::kChromeUIFydeAssistantAppURL);
  info->scope = GURL(ash::kChromeUIFydeAssistantAppURL);
  info->title = u"FydeOS AI";
  web_app::CreateIconInfoForSystemWebApp(
      info->start_url,
      {{"app_icon_192.png", 192, IDR_ASH_FYDE_ASSISTANT_APP_APP_ICON_192_PNG}},
      *info);
  info->display_mode = blink::mojom::DisplayMode::kStandalone;
  info->user_display_mode = web_app::mojom::UserDisplayMode::kStandalone;

  return info;
}

FydeAssistantAppDelegate::FydeAssistantAppDelegate(Profile* profile)
    : ash::SystemWebAppDelegate(ash::SystemWebAppType::FYDE_ASSISTANT,
                                    "FydeAssistant",
                                    GURL(ash::kChromeUIFydeAssistantAppURL),
                                    profile) {}

bool FydeAssistantAppDelegate::ShouldCaptureNavigations() const  {
  return true;
}

std::unique_ptr<WebAppInstallInfo> FydeAssistantAppDelegate::GetWebAppInfo() const {
  return CreateWebAppInfoForFydeAssistantApp();
}
bool FydeAssistantAppDelegate::IsAppEnabled() const {
  return ash::features::IsFydeAssistantEnabled();
}
