#include "chrome/browser/ash/web_applications/fyde_assistant_app_ui_delegate.h"
#include "content/public/browser/web_ui.h"
#include "content/public/browser/web_ui_data_source.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ash/system_web_apps/color_helpers.h"
#include "ui/color/color_provider_utils.h"
#include "ui/native_theme/native_theme.h"
#include "ui/chromeos/styles/cros_tokens_color_mappings.h"
#include "ui/color/color_provider.h"

ChromeFydeAssistantAppUIDelegate::ChromeFydeAssistantAppUIDelegate(content::WebUI* web_ui)
    : web_ui_(web_ui) {}

ChromeFydeAssistantAppUIDelegate::~ChromeFydeAssistantAppUIDelegate() = default;

void ChromeFydeAssistantAppUIDelegate::PopulateLoadTimeData(content::WebUIDataSource* source) {
  Profile* profile = Profile::FromWebUI(web_ui_);
  source->AddString("user", profile->GetProfileUserName());

  auto colors = GetSystemColorInfo();
  source->AddString("base_color", ui::ConvertSkColorToCSSColor(colors.base));
  source->AddString("shaded_color", ui::ConvertSkColorToCSSColor(colors.shaded));
  source->AddString("header_color", ui::ConvertSkColorToCSSColor(colors.header));
  source->AddString("primary_color", ui::ConvertSkColorToCSSColor(colors.primary));
}

ash::FydeAssistantAppUIDelegate::ColorInfo ChromeFydeAssistantAppUIDelegate::GetSystemColorInfo() {
  auto* native_theme = ui::NativeTheme::GetInstanceForNativeUi();
  auto* color_provider = ui::ColorProviderManager::Get().GetColorProviderFor(
      native_theme->GetColorProviderKey(nullptr));
  return {
   color_provider->GetColor(cros_tokens::kCrosSysAppBase),
   color_provider->GetColor(cros_tokens::kCrosSysAppBaseShaded),
   color_provider->GetColor(cros_tokens::kCrosSysHeader),
   color_provider->GetColor(cros_tokens::kCrosSysPrimary),
  };
}
