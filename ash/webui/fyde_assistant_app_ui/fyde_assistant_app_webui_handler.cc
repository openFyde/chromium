#include "ash/webui/fyde_assistant_app_ui/fyde_assistant_app_webui_handler.h"
#include "ash/public/cpp/new_window_delegate.h"
#include "ash/public/cpp/assistant/controller/assistant_ui_controller.h"
#include "ash/app_list/app_list_controller_impl.h"
#include "ash/assistant/util/deep_link_util.h"
#include "ash/assistant/model/assistant_ui_model.h"
#include "ash/shell.h"
#include "chrome/browser/ash/system_web_apps/color_helpers.h"
#include "ui/color/color_provider_utils.h"

namespace ash {

FydeAssistantWebUIHandler::FydeAssistantWebUIHandler(FydeAssistantAppUI* app_ui):fyde_assistant_app_ui_(app_ui) {
  assistant_controller_observation_.Observe(AssistantController::Get());
  theme_observation_.Observe(ui::NativeTheme::GetInstanceForNativeUi());
  AssistantUiController::Get()->GetModel()->AddObserver(this);
  Shelf* shelf = Shelf::ForWindow(Shell::GetPrimaryRootWindow());
  if  (shelf && shelf->fyde_assistant_view()) {
    shelf->fyde_assistant_view()->AddObserver(this);
  }
}

FydeAssistantWebUIHandler::~FydeAssistantWebUIHandler() {
  if (AssistantUiController::Get())
    AssistantUiController::Get()->GetModel()->RemoveObserver(this);
  Shelf* shelf = Shelf::ForWindow(Shell::GetPrimaryRootWindow());
  if  (shelf && shelf->fyde_assistant_view()) {
    shelf->fyde_assistant_view()->RemoveObserver(this);
  }
}

void FydeAssistantWebUIHandler::RegisterMessages() {
  web_ui()->RegisterMessageCallback(
      "onFydeAssistantSwaInit",
      base::BindRepeating(&FydeAssistantWebUIHandler::OnFydeAssistantSwaInit,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "onCloseAssistant",
      base::BindRepeating(&FydeAssistantWebUIHandler::OnRequestCloseAssistant,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "onOpenAssistantUrl",
      base::BindRepeating(&FydeAssistantWebUIHandler::OnFydeAssistantOpenUrl,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "setAssistantBubbleRect",
      base::BindRepeating(&FydeAssistantWebUIHandler::HandleSetAssistantBubbleRect,
                          base::Unretained(this)));
}

void FydeAssistantWebUIHandler::OnFydeAssistantSwaInit(const base::Value::List& args) {
  AllowJavascript();
}

void FydeAssistantWebUIHandler::OnRequestCloseAssistant(const base::Value::List& args) {
  CHECK_EQ(1u, args.size());
  const std::string source = args[0].GetString();
  if (source == "launcher") {
    auto* const app_list_controller = Shell::Get()->app_list_controller();
    if (app_list_controller) {
      app_list_controller->CloseAssistant();
    }
    return;
  }

  if (source == "bubble") {
    Shelf* shelf = Shelf::ForWindow(Shell::GetPrimaryRootWindow());
    if  (shelf && shelf->fyde_assistant_view()) {
      shelf->fyde_assistant_view()->HideBubble();
    }
  }
}

void FydeAssistantWebUIHandler::OnFydeAssistantOpenUrl(const base::Value::List& args) {
  CHECK_EQ(1u, args.size());
  const std::string url = args[0].GetString();
  ash::NewWindowDelegate::GetPrimary()->OpenUrl(
    GURL(url), ash::NewWindowDelegate::OpenUrlFrom::kUserInteraction,
    ash::NewWindowDelegate::Disposition::kNewWindow);
}

void FydeAssistantWebUIHandler::HandleSetAssistantBubbleRect(const base::Value::List& args) {
  CHECK_EQ(4u, args.size());
  const int x = args[0].GetInt();
  const int y = args[1].GetInt();
  const int width = args[2].GetInt();
  const int height = args[3].GetInt();
  Shelf* shelf = Shelf::ForWindow(Shell::GetPrimaryRootWindow());
  if  (shelf && shelf->fyde_assistant_view()) {
    shelf->fyde_assistant_view()->SetBubbleRect(x, y, width, height);
  }
}

void FydeAssistantWebUIHandler::OnDeepLinkReceived(
    assistant::util::DeepLinkType type,
    const std::map<std::string, std::string>& params) {
  if (!IsJavascriptAllowed()) {
    return;
  }
  if (type == assistant::util::DeepLinkType::kQuery) {
    const absl::optional<std::string>& query =
      GetDeepLinkParam(params, assistant::util::DeepLinkParam::kQuery);
    if (query.has_value() && IsJavascriptAllowed()) {
      FireWebUIListener("query-from-launcher", base::Value(query.value()));
    }
  }
}

void FydeAssistantWebUIHandler::OnUiVisibilityChanged(
      AssistantVisibility new_visibility,
      AssistantVisibility old_visibility,
      absl::optional<AssistantEntryPoint> entry_point,
      absl::optional<AssistantExitPoint> exit_point) {
  if (!IsJavascriptAllowed()) {
    return;
  }
  if (new_visibility == AssistantVisibility::kVisible || new_visibility == AssistantVisibility::kClosed) {
    base::Value::Dict value;
    value.Set("visible", new_visibility == AssistantVisibility::kVisible);
    value.Set("entry", static_cast<int>(entry_point.has_value() ? entry_point.value() : AssistantEntryPoint::kUnspecified));
    FireWebUIListener("ui-visibility-changed", value);
  }
}

void FydeAssistantWebUIHandler::OnNativeThemeUpdated(ui::NativeTheme* observed_theme) {
  if (!IsJavascriptAllowed()) {
    return;
  }
  auto colors = fyde_assistant_app_ui_->delegate()->GetSystemColorInfo();
  base::Value::Dict value;
  value.Set("base", ui::ConvertSkColorToCSSColor(colors.base));
  value.Set("shaded", ui::ConvertSkColorToCSSColor(colors.shaded));
  value.Set("header", ui::ConvertSkColorToCSSColor(colors.header));
  value.Set("primary", ui::ConvertSkColorToCSSColor(colors.primary));
  FireWebUIListener("system-color-changed", value);
}

void FydeAssistantWebUIHandler::OnBubbleQueryChanged(const FydeAssistantViewObserver::ClipboardItemForAssistant& item) {
  if (!IsJavascriptAllowed()) {
    return;
  }
  if (item.display_text.size() > 0) {
    base::Value::Dict value;
    value.Set("query", item.display_text);
    value.Set("format", item.display_format);
    FireWebUIListener("query-from-bubble", value);
  }
}

void FydeAssistantWebUIHandler::OnBubbleVisibilityChanged(bool visible)  {
  if (!IsJavascriptAllowed()) {
    return;
  }
  FireWebUIListener("bubble-visibility-changed", base::Value(visible));
}

} // namespace ash
