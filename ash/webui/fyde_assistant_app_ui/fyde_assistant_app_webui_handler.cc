#include "ash/webui/fyde_assistant_app_ui/fyde_assistant_app_webui_handler.h"
#include "ash/public/cpp/new_window_delegate.h"
#include "ash/app_list/app_list_controller_impl.h"
#include "ash/assistant/util/deep_link_util.h"
#include "ash/shell.h"

namespace ash {

FydeAssistantWebUIHandler::FydeAssistantWebUIHandler() {
  assistant_controller_observation_.Observe(AssistantController::Get());
}

FydeAssistantWebUIHandler::~FydeAssistantWebUIHandler() = default;

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
}

void FydeAssistantWebUIHandler::OnFydeAssistantSwaInit(const base::Value::List& args) {
  AllowJavascript();
}

void FydeAssistantWebUIHandler::OnRequestCloseAssistant(const base::Value::List& args) {
  auto* const app_list_controller = Shell::Get()->app_list_controller();
  if (app_list_controller) {
    app_list_controller->CloseFydeAssistant();
  }
}

void FydeAssistantWebUIHandler::OnFydeAssistantOpenUrl(const base::Value::List& args) {
  CHECK_EQ(1u, args.size());
  const std::string url = args[0].GetString();
  ash::NewWindowDelegate::GetPrimary()->OpenUrl(
    GURL(url), ash::NewWindowDelegate::OpenUrlFrom::kUserInteraction,
    ash::NewWindowDelegate::Disposition::kNewWindow);
}

void FydeAssistantWebUIHandler::OnDeepLinkReceived(
    assistant::util::DeepLinkType type,
    const std::map<std::string, std::string>& params) {
  if (!IsJavascriptAllowed()) {
    return;
  }
  if (type == assistant::util::DeepLinkType::kQuery) {
    const std::optional<std::string>& query =
      GetDeepLinkParam(params, assistant::util::DeepLinkParam::kQuery);
    if (query.has_value() && IsJavascriptAllowed()) {
      FireWebUIListener("query-from-launcher", base::Value(query.value()));
    }
  }
}

} // namespace ash
