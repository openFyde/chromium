#include "ash/webui/fyde_assistant_app_ui/fyde_assistant_app_ui.h"

#include "ash/webui/fyde_assistant_app_ui/url_constants.h"
#include "chrome/browser/ash/system_web_apps/apps/system_web_app_install_utils.h"
#include "ash/webui/grit/ash_fyde_assistant_app_resources.h"
#include "ash/webui/grit/ash_fyde_assistant_app_resources_map.h"

#include "content/public/browser/web_ui_data_source.h"
#include "content/public/browser/web_ui.h"
#include "content/public/browser/web_contents.h"

#include "ash/bubble/bubble_constants.h"
#include "ash/session/session_controller_impl.h"
#include "ash/shell.h"

#include "content/public/browser/web_ui_message_handler.h"
#include "ash/webui/fyde_assistant_app_ui/fyde_assistant_app_webui_handler.h"
#include "components/prefs/pref_service.h"
#include "fydeos/prefs/fydeos_pref_names.h"
#include "fydeos/switches/services/services_switches.h"

namespace ash {

FydeAssistantAppUI::FydeAssistantAppUI(content::WebUI* web_ui,
                                      std::unique_ptr<FydeAssistantAppUIDelegate> delegate)
  : content::WebUIController(web_ui), delegate_(std::move(delegate)) {
  auto* browser_context = web_ui->GetWebContents()->GetBrowserContext();
  content::WebUIDataSource* html_source =
      content::WebUIDataSource::CreateAndAdd(browser_context,
                                             kChromeUIFydeAssistantAppHost);
  html_source->OverrideContentSecurityPolicy(
      network::mojom::CSPDirectiveName::ScriptSrc,
      "script-src chrome://resources chrome://test chrome://webui-test "
      "'self';");
  html_source->OverrideContentSecurityPolicy(
      network::mojom::CSPDirectiveName::WorkerSrc,
    "worker-src blob: chrome://resources 'self';");
  html_source->DisableTrustedTypesCSP();
  html_source->AddResourcePath("", IDR_ASH_FYDE_ASSISTANT_APP_INDEX_HTML);
  html_source->AddResourcePaths(base::span(kAshFydeAssistantAppResources));

  html_source->SetDefaultResource(IDR_ASH_FYDE_ASSISTANT_APP_INDEX_HTML);

  html_source->AddInteger("borderRadiusInLauncher", kBubbleCornerRadius);
  html_source->AddInteger("borderRadiusInBubble", kBubbleCornerRadiusForAI);
  html_source->AddString("fydeosAssistantUrl", fydeos::switches::GetFydeOSAssistantWebUrl());
  bool fyde_assistant_enabled = false;
  PrefService* prefs =
      Shell::Get()->session_controller()->GetActivePrefService();
  if (prefs &&
      prefs->FindPreference(fydeos::prefs::kFydeAssistantEnabled)) {
    fyde_assistant_enabled =
        prefs->GetBoolean(fydeos::prefs::kFydeAssistantEnabled);
  }
  html_source->AddBoolean("isFydeOSAssistantEnabled",
                          fyde_assistant_enabled);
  html_source->UseStringsJs();

  delegate_->PopulateLoadTimeData(html_source);

  web_ui->AddMessageHandler(std::make_unique<FydeAssistantWebUIHandler>(this));
}

FydeAssistantAppUI::~FydeAssistantAppUI() = default;

WEB_UI_CONTROLLER_TYPE_IMPL(FydeAssistantAppUI)

} // namespace ash
