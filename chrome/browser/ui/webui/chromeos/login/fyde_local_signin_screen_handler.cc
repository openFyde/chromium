#include "chrome/browser/ui/webui/chromeos/login/fyde_local_signin_screen_handler.h"

#include "chrome/browser/ash/login/oobe_screen.h"
#include "chrome/browser/ash/login/screens/fyde_local_signin_screen.h"

namespace chromeos {

constexpr StaticOobeScreenId FydeLocalSigninView::kScreenId;

FydeLocalSigninScreenHandler::FydeLocalSigninScreenHandler()
    : BaseScreenHandler(kScreenId) {}

FydeLocalSigninScreenHandler::~FydeLocalSigninScreenHandler() =
    default;

void FydeLocalSigninScreenHandler::RegisterMessages() {
    BaseScreenHandler::RegisterMessages();
    AddCallback("completeFtAuthentication",
                &FydeLocalSigninScreenHandler::HandleCompleteAuth);
}

void FydeLocalSigninScreenHandler::DeclareLocalizedValues(
    ::login::LocalizedValuesBuilder* builder) {
}

void FydeLocalSigninScreenHandler::Show() {
  base::Value::Dict data;
  data.Set("emailDomain", "fydeos.local");
  ShowInWebUI(std::move(data));
}

void FydeLocalSigninScreenHandler::HandleCompleteAuth(const std::string& username,
                                                      const std::string& password) {
}

void FydeLocalSigninScreenHandler::Reset() {
    CallExternalAPI("reset");
}

void FydeLocalSigninScreenHandler::SetErrorState(
    const std::string& username,
    int errorState) {
    CallExternalAPI("setErrorState", username, errorState);
}

}
