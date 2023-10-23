// Copyright 2022 Fyde Innovations. All rights reserved

#include "chrome/browser/ui/webui/ash/login/fyde_local_signin_screen_handler.h"

#include "chrome/browser/browser_process.h"
#include "chrome/browser/ash/login/oobe_screen.h"
#include "chrome/browser/ash/login/screens/fyde_local_signin_screen.h"
#include "chrome/grit/generated_resources.h"
#include "components/login/localized_values_builder.h"
#include "chrome/browser/ash/login/ui/login_display_host.h"
#include "chrome/browser/ash/login/existing_user_controller.h"
#include "chrome/browser/ash/settings/cros_settings.h"
#include "chromeos/ash/components/login/auth/public/cryptohome_key_constants.h"
#include "components/user_manager/known_user.h"
#include "chromeos/ash/components/login/auth/public/key.h"

namespace ash {

constexpr StaticOobeScreenId FydeLocalSigninView::kScreenId;

namespace {

enum class FYDE_LOCAL_SIGNIN_ERROR_STATE {
  NONE = 0,
  BAD_USERNAME = 1,
  BAD_AUTH_PASSWORD = 2,
  BAD_CONFIRM_PASSWORD = 3,
  BAD_USERNAME_OR_PASSWORD_ERROR = 4,
};

std::string CreateFydeLocalAccountID(const std::string& username) {
  std::string::size_type pos;
  pos = username.find("@");
  return (pos == std::string::npos ? username : username.substr(0, pos));
}

}  // namespace

FydeLocalSigninScreenHandler::FydeLocalSigninScreenHandler()
    : BaseScreenHandler(kScreenId) {}

FydeLocalSigninScreenHandler::~FydeLocalSigninScreenHandler() =
    default;

void FydeLocalSigninScreenHandler::DeclareJSCallbacks() {
    AddCallback("completeFtAuthentication",
                &FydeLocalSigninScreenHandler::HandleCompleteAuth);
}

void FydeLocalSigninScreenHandler::DeclareLocalizedValues(
    ::login::LocalizedValuesBuilder* builder) {
  builder->Add("fydeosLocalSignupTitle", IDS_FYDEOS_LOCAL_SIGNUP_TITLE);
  builder->Add("fydeosLocalSigninTitle", IDS_FYDEOS_LOCAL_SIGNIN_TITLE);
  builder->Add("fydeosLocalSigninUsername", IDS_FYDEOS_LOCAL_SIGNIN_USERNAME);
  builder->Add("fydeosLocalSigninInvalidUsername",
      IDS_FYDEOS_LOCAL_SIGNIN_INVALID_USERNAME);
  builder->Add("fydeosLocalSigninPassword", IDS_FYDEOS_LOCAL_SIGNIN_PASSWORD);
  builder->Add("fydeosLocalSigninInvalidPassword",
      IDS_FYDEOS_LOCAL_SIGNIN_INVALID_PASSWORD);
  builder->Add("fydeosLocalSigninPasswordConfirm",
      IDS_FYDEOS_LOCAL_SIGNIN_PASSWORD_CONFIRM);
  builder->Add("fydeosLocalSigninPasswordConfirmError",
      IDS_FYDEOS_LOCAL_SIGNIN_PASSWORD_CONFIRM_ERROR);
  builder->Add("fydeosLocalSigninNewLocalAccountButtonText",
      IDS_FYDEOS_LOCAL_SIGNIN_NEW_LOCAL_ACCOUNT_BUTTON_TEXT);
  builder->Add("fydeosLocalSigninExistLocalAccountButtonText",
      IDS_FYDEOS_LOCAL_SIGNIN_EXIST_LOCAL_ACCOUNT_BUTTON_TEXT);
  builder->Add("fydeosLocalSigninExistLocalAccountErrorMessage",
      IDS_FYDEOS_LOCAL_SIGNIN_EXIST_LOCAL_ACCOUNT_ERROR_MESSAGE);
}

void FydeLocalSigninScreenHandler::Show() {
  base::Value::Dict data;
  data.Set("emailDomain", "fydeos.local");
  bool show_users_on_signin;
  ash::CrosSettings::Get()->GetBoolean(
      ash::kAccountsPrefShowUserNamesOnSignIn, &show_users_on_signin);
  data.Set("showUsersOnSignin", show_users_on_signin);
  ShowInWebUI(std::move(data));
}

void FydeLocalSigninScreenHandler::HandleCompleteAuth(
    const bool newUser,
    const std::string& username, const std::string& password) {
  if (username.empty()) {
    SetErrorState(username,
        static_cast<int>(FYDE_LOCAL_SIGNIN_ERROR_STATE::BAD_USERNAME));
    return;
  }
  if (password.empty()) {
    SetErrorState(username,
        static_cast<int>(FYDE_LOCAL_SIGNIN_ERROR_STATE::BAD_AUTH_PASSWORD));
    return;
  }
  bool exist = false;
  user_manager::KnownUser known_user(g_browser_process->local_state());
  const std::vector<AccountId> known_account_ids =
    known_user.GetKnownAccountIds();
  for (const AccountId& known_id : known_account_ids) {
    if (known_id.GetUserEmail() == username) {
      exist = true;
      break;
    }
  }

  if (exist && newUser) {
    // signup an existing account
    SetErrorState(username,
        static_cast<int>(FYDE_LOCAL_SIGNIN_ERROR_STATE::BAD_USERNAME));
    return;
  } else if (!exist && !newUser) {
    // signin an non-existent account
    SetErrorState(username, static_cast<int>(
          FYDE_LOCAL_SIGNIN_ERROR_STATE::BAD_USERNAME_OR_PASSWORD_ERROR));
    return;
  }

  if (LoginDisplayHost::default_host())
    LoginDisplayHost::default_host()->SetDisplayEmail(username);

  Key key(password);
  key.SetLabel(kCryptohomeGaiaKeyLabel);
  DoCompleteLogin(newUser, username, key);
}

void FydeLocalSigninScreenHandler::DoCompleteLogin(const bool newUser,
                                                   const std::string& username,
                                                   const Key& key) {
  std::string userId = CreateFydeLocalAccountID(username);
  user_manager::KnownUser known_user(g_browser_process->local_state());
  const AccountId account_id(known_user.GetAccountId(
        username, "ft_id_" + userId , AccountType::FLINT_ACCOUNT));
  LoginDisplayHost::default_host()->SetDisplayAndGivenName(
      userId, userId);
  UserContext user_context(
      user_manager::UserType::USER_TYPE_FLINT_ACCOUNT, account_id);
  user_context.SetKey(key);
  user_context.SetAuthFlow(UserContext::AUTH_FLOW_FLINT_ACCOUNT);
  user_context.SetIsUsingOAuth(false);
  if (newUser) {
    LoginDisplayHost::default_host()->CompleteLogin(user_context);
  } else {
    if (ExistingUserController::current_controller()) {
      ExistingUserController::current_controller()->Login(user_context,
                                                          SigninSpecifics());
    } else {
      LOG(ERROR) << "FydeLocalSigninScreenHandler::DoCompleteLogin: "
                 << "ExistingUserController not available.";
      SetErrorState(username, static_cast<int>(
            FYDE_LOCAL_SIGNIN_ERROR_STATE::BAD_USERNAME_OR_PASSWORD_ERROR));
    }
  }
}

void FydeLocalSigninScreenHandler::Reset() {
    CallExternalAPI("reset");
}

void FydeLocalSigninScreenHandler::SetErrorState(
    const std::string& username,
    int errorState) {
    CallExternalAPI("setErrorState", username, errorState);
}

}  // namespace ash
