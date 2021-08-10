#include "chrome/browser/ui/webui/chromeos/login/fyde_local_signin_screen_handler.h"

#include "chrome/browser/ash/login/oobe_screen.h"
#include "chrome/browser/ash/login/screens/fyde_local_signin_screen.h"

namespace chromeos {

constexpr StaticOobeScreenId FydeLocalSigninView::kScreenId;

FydeLocalSigninScreenHandler::FydeLocalSigninScreenHandler(
    JSCallsContainer* js_calls_container)
    : BaseScreenHandler(kScreenId, js_calls_container) {
  set_user_acted_method_path("login.FydeLocalSigninScreen.userActed");
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

void FydeLocalSigninScreenHandler::Initialize() {
  if (show_on_init_) {
    show_on_init_ = false;
    Show();
  }
}

void FydeLocalSigninScreenHandler::Show() {
    if (!page_is_ready()) {
        show_on_init_ = true;
        return;
    }
    base::DictionaryValue screen_data;
    screen_data.SetString("emailDomain", "fydeos.local");
    ShowScreenWithData(kScreenId, &screen_data);
}

void FydeLocalSigninScreenHandler::Bind(
    FydeLocalSigninScreen* screen) {
    screen_ = screen;
    BaseScreenHandler::SetBaseScreen(screen_);
}

void FydeLocalSigninScreenHandler::Unbind() {
    screen_ = nullptr;
    BaseScreenHandler::SetBaseScreen(nullptr);
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
