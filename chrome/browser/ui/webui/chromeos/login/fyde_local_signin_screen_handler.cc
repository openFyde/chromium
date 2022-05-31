#include "chrome/browser/ui/webui/chromeos/login/fyde_local_signin_screen_handler.h"

#include "chrome/browser/ash/login/oobe_screen.h"
#include "chrome/browser/ash/login/screens/fyde_local_signin_screen.h"

namespace chromeos {

constexpr StaticOobeScreenId FydeLocalSigninView::kScreenId;

FydeLocalSigninScreenHandler::FydeLocalSigninScreenHandler()
    : BaseScreenHandler(kScreenId) {
  set_user_acted_method_path_deprecated("login.FydeLocalSigninScreen.userActed");
}

FydeLocalSigninScreenHandler::~FydeLocalSigninScreenHandler() {
    if (screen_) {
        screen_->OnViewDestroyed(this);
    }
}

void FydeLocalSigninScreenHandler::RegisterMessages() {
    BaseScreenHandler::RegisterMessages();
    AddCallback("completeFtAuthentication",
                &FydeLocalSigninScreenHandler::HandleCompleteAuth);
}

void FydeLocalSigninScreenHandler::DeclareLocalizedValues(
    ::login::LocalizedValuesBuilder* builder) {

}

void FydeLocalSigninScreenHandler::InitializeDeprecated() {
  if (show_on_init_) {
    show_on_init_ = false;
    Show();
  }
}

void FydeLocalSigninScreenHandler::Show() {
  if (!IsJavascriptAllowed()) {
    show_on_init_ = true;
    return;
  }
  base::Value::Dict data;
  data.Set("emailDomain", "fydeos.local");
  ShowInWebUI(std::move(data));
}

void FydeLocalSigninScreenHandler::Bind(
    FydeLocalSigninScreen* screen) {
    screen_ = screen;
    BaseScreenHandler::SetBaseScreenDeprecated(screen_);
}

void FydeLocalSigninScreenHandler::Unbind() {
    screen_ = nullptr;
    BaseScreenHandler::SetBaseScreenDeprecated(nullptr);
}

void FydeLocalSigninScreenHandler::HandleCompleteAuth(const std::string& username,
                                                      const std::string& password) {
}

void FydeLocalSigninScreenHandler::Reset() {
    CallJS("login.FydeLocalSigninScreen.reset");
}

void FydeLocalSigninScreenHandler::SetErrorState(
    const std::string& username,
    int errorState) {
    CallJS("login.FydeLocalSigninScreen.setErrorState", username,
           errorState);
}

}
