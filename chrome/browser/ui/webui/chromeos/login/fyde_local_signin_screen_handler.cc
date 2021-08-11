#include "chrome/browser/ui/webui/chromeos/login/fyde_local_signin_screen_handler.h"

#include "chrome/browser/ash/login/oobe_screen.h"
#include "chrome/browser/ash/login/screens/fyde_local_signin_screen.h"
#include "chrome/grit/generated_resources.h"
#include "components/login/localized_values_builder.h"
#include "chrome/browser/ash/login/ui/login_display_host.h"
#include "ash/components/login/auth/cryptohome_key_constants.h"
#include "components/user_manager/known_user.h"
#include "ash/components/login/auth/key.h"

namespace chromeos {

constexpr StaticOobeScreenId FydeLocalSigninView::kScreenId;

namespace {

enum class FYDE_LOCAL_SIGNIN_ERROR_STATE {
  NONE = 0,
  BAD_USERNAME = 1,
  BAD_AUTH_PASSWORD = 2,
  BAD_CONFIRM_PASSWORD = 3,
};

std::string CreateFydeLocalAccountID(const std::string& username){
  std::string::size_type pos;
  pos = username.find("@");
  return (pos == std::string::npos ? username : username.substr(0, pos));
}

}

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
  builder->Add("fydeosLocalSigninTitle", IDS_FYDEOS_LOCAL_SIGNIN_TITLE);
  builder->Add("fydeosLocalSigninUsername", IDS_FYDEOS_LOCAL_SIGNIN_USERNAME);
  builder->Add("fydeosLocalSigninInvalidUsername", IDS_FYDEOS_LOCAL_SIGNIN_INVALID_USERNAME);
  builder->Add("fydeosLocalSigninPassword", IDS_FYDEOS_LOCAL_SIGNIN_PASSWORD);
  builder->Add("fydeosLocalSigninInvalidPassword", IDS_FYDEOS_LOCAL_SIGNIN_INVALID_PASSWORD);
  builder->Add("fydeosLocalSigninPasswordConfirm", IDS_FYDEOS_LOCAL_SIGNIN_PASSWORD_CONFIRM);
  builder->Add("fydeosLocalSigninPasswordConfirmError", IDS_FYDEOS_LOCAL_SIGNIN_PASSWORD_CONFIRM_ERROR);
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
  if (username.empty()) {
    SetErrorState(username, static_cast<int>(FYDE_LOCAL_SIGNIN_ERROR_STATE::BAD_USERNAME));
    return;
  }
  if (password.empty()) {
    SetErrorState(username, static_cast<int>(FYDE_LOCAL_SIGNIN_ERROR_STATE::BAD_AUTH_PASSWORD));
    return;
  }
  const std::vector<AccountId> known_account_ids =
      user_manager::known_user::GetKnownAccountIds();
  for (const AccountId& known_id : known_account_ids) {
    if (known_id.GetUserEmail() == username) {
      SetErrorState(username, static_cast<int>(FYDE_LOCAL_SIGNIN_ERROR_STATE::BAD_USERNAME));
      return;
    }
  }

  if (LoginDisplayHost::default_host())
    LoginDisplayHost::default_host()->SetDisplayEmail(username);

  Key key(password);
  key.SetLabel(kCryptohomeGaiaKeyLabel);
  DoCompleteLogin(username, key);
}

void FydeLocalSigninScreenHandler::DoCompleteLogin(const std::string& username,
                                                   const Key& key) {
  std::string userId = CreateFydeLocalAccountID(username);
  const AccountId account_id(user_manager::known_user::GetAccountId(
        username, "ft_id_" + userId , AccountType::FLINT_ACCOUNT));
  LoginDisplayHost::default_host()->SetDisplayAndGivenName(
      userId, userId);
  UserContext user_context(user_manager::UserType::USER_TYPE_FLINT_ACCOUNT, account_id);
  user_context.SetKey(key);
  user_context.SetAuthFlow(UserContext::AUTH_FLOW_FLINT_ACCOUNT);
  user_context.SetIsUsingOAuth(false);
  LoginDisplayHost::default_host()->CompleteLogin(user_context);
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
