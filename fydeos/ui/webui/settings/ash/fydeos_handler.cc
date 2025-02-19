// Copyright (c) 2021 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fydeos/ui/webui/settings/ash/fydeos_handler.h"

#include "base/logging.h"
#include "base/values.h"
#include "ash/shell.h"
#include "chrome/browser/profiles/profile.h"
#include "components/prefs/pref_service.h"
#include "chrome/browser/browser_process.h"
#include "components/user_manager/user_manager.h"
#include "ash/wm/tablet_mode/tablet_mode_controller.h"
#include "fydeos/prefs/fydeos_pref_names.h"
#include "chrome/browser/ash/profiles/profile_helper.h"
// #include "chromeos/cryptohome/system_salt_getter.h"
// #include "chrome/browser/ash/settings/token_encryptor.h"

namespace ash::settings {

// FydeOsHandler::FydeOsHandler(Profile* profile, PrefService* prefs) :
//   profile_(profile), prefs_(prefs) {}
FydeOsHandler::FydeOsHandler(Profile* profile, PrefService* prefs) :
  profile_(profile), prefs_(prefs) {
  DCHECK(ash::Shell::Get());
  ash::Shell::Get()->tablet_mode_controller()->AddObserver(this);
  // TODO(fangzhou) use real system_salt_ and encryptor
  // SystemSaltGetter::Get()->GetSystemSalt(base::BindOnce(
  //     &FydeOsHandler::OnSystemSaltObtained, weak_ptr_factory_.GetWeakPtr()));
  OnSystemSaltObtained("FYDEOS");
}

FydeOsHandler::~FydeOsHandler() {
  if (ash::Shell::Get()->tablet_mode_controller())
    ash::Shell::Get()->tablet_mode_controller()->RemoveObserver(this);
}

void FydeOsHandler::RegisterMessages() {
  web_ui()->RegisterMessageCallback(
      "getIsOfflineAutoSigninEnabled",
      base::BindRepeating(&FydeOsHandler::HandleGetIsOfflineAutoSigninEnabled,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
               "saveOfflineLoginPassword",
                base::BindRepeating(&FydeOsHandler::HandleSaveOfflineLoginPassword,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
               "cleanOfflineLoginPassword",
                base::BindRepeating(&FydeOsHandler::HandleCleanOfflineLoginPassword,
                          base::Unretained(this)));

  web_ui()->RegisterMessageCallback(
      "getShowRotateScreenButton",
      base::BindRepeating(&FydeOsHandler::HandleGetShowRotateScreenButton,
                          base::Unretained(this)));

  web_ui()->RegisterMessageCallback(
      "setShowRotateScreenButton",
      base::BindRepeating(&FydeOsHandler::HandleSetShowRotateScreenButton,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "getIsInTabletPhysicalState",
      base::BindRepeating(&FydeOsHandler::HandleGetIsInTabletPhysicalState,
                          base::Unretained(this)));

  web_ui()->RegisterMessageCallback(
      "getShowSwitchTabletLaptopButton",
      base::BindRepeating(&FydeOsHandler::HandleGetShowSwitchTabletLaptopButton,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "setShowSwitchTabletLaptopButton",
      base::BindRepeating(&FydeOsHandler::HandleSetShowSwitchTabletLaptopButton,
                          base::Unretained(this)));

  web_ui()->RegisterMessageCallback(
      "getIsForceTpmFallback",
      base::BindRepeating(&FydeOsHandler::HandleGetIsForceTpmFallback,
                          base::Unretained(this)));

  web_ui()->RegisterMessageCallback(
      "setForceTpmFallback",
      base::BindRepeating(&FydeOsHandler::HandleSetForceTpmFallback,
                          base::Unretained(this)));
}

void FydeOsHandler::OnJavascriptAllowed() {
  pref_change_registrar_.Init(prefs_);

  local_state_pref_change_registrar_.Init(g_browser_process->local_state());
  local_state_pref_change_registrar_.Add(
      fydeos::prefs::kShowRotateScreenButton,
      base::BindRepeating(
          &FydeOsHandler::OnShowRotateScreenButtonChanged,
          base::Unretained(this)));
  local_state_pref_change_registrar_.Add(
      fydeos::prefs::kShowSwitchTabletLaptopButton,
      base::BindRepeating(
          &FydeOsHandler::OnShowSwitchTabletLaptopButtonChanged,
          base::Unretained(this)));
  local_state_pref_change_registrar_.Add(
      fydeos::prefs::kForceTpmFallback,
      base::BindRepeating(
          &FydeOsHandler::OnForceTpmFallbackChanged,
          base::Unretained(this)));
}

void FydeOsHandler::OnJavascriptDisallowed() {
  pref_change_registrar_.RemoveAll();
  local_state_pref_change_registrar_.RemoveAll();
}

void FydeOsHandler::OnShowRotateScreenButtonChanged() {
  PrefService* prefs = g_browser_process->local_state();
  bool showRotate = prefs->GetBoolean(fydeos::prefs::kShowRotateScreenButton);
  FireWebUIListener("show-rotate-screen-button-changed",
                    base::Value(showRotate));
}

void FydeOsHandler::OnSystemSaltObtained(const std::string& system_salt) {
  system_salt_ = system_salt;
  if (IsJavascriptAllowed()) {
    FireWebUIListener("offline-auto-signin-system-salt-obtained");
  }
}

void FydeOsHandler::HandleGetIsOfflineAutoSigninEnabled(
    const base::Value::List& args) {
  AllowJavascript();
  CHECK(args.size());
  const base::Value& callback_id = args[0];
  PrefService* prefs = g_browser_process->local_state();
  const std::string& password =
    prefs->GetString(fydeos::prefs::kOfflineAutoSigninPassword);
  const std::string& account_id_key =
    prefs->GetString(fydeos::prefs::kOfflineAutoSigninAccountIdKey);
  base::Value::Dict response;
  response.Set("enabled", !account_id_key.empty() && !password.empty());
  const user_manager::User* user =
      ProfileHelper::Get()->GetUserByProfile(profile_);
  if (!user->GetAccountId().HasAccountIdKey() || !user->IsFlintAccountUser()) {
    response.Set("is_current_user", false);
  }
  if (user->GetAccountId().GetAccountIdKey() == account_id_key) {
    response.Set("is_current_user", true);
  } else {
    response.Set("is_current_user", false);
  }
  response.Set("system_salt_obtained", !system_salt_.empty());
  ResolveJavascriptCallback(callback_id, response);
}

void FydeOsHandler::HandleSaveOfflineLoginPassword(
    const base::Value::List& args) {
  CHECK_EQ(2u, args.size());
  const base::Value& callback_id = args[0];
  if (!g_browser_process || system_salt_.empty()) {
    ResolveJavascriptCallback(callback_id, base::Value(false));
    return;
  }
  std::string password = args[1].GetString();
  if (password.empty()) {
    ResolveJavascriptCallback(callback_id, base::Value(false));
  }
  user_manager::User* user =
    ash::ProfileHelper::Get()->GetUserByProfile(profile_);
  const AccountId account_id = user->GetAccountId();
  if (!account_id.HasAccountIdKey()) {
    ResolveJavascriptCallback(callback_id, base::Value(false));
    return;
  }
  PrefService* prefs = g_browser_process->local_state();
  // ash::CryptohomeTokenEncryptor encryptor(system_salt_);
  // prefs->SetString(fydeos::prefs::kOfflineAutoSigninPassword,
  //    encryptor.EncryptWithSystemSalt(password));
  prefs->SetString(fydeos::prefs::kOfflineAutoSigninPassword, password);
  prefs->SetString(fydeos::prefs::kOfflineAutoSigninPasswordFormat,
      "plaintext");
  prefs->SetString(fydeos::prefs::kOfflineAutoSigninAccountIdKey,
      account_id.GetAccountIdKey());
  ResolveJavascriptCallback(callback_id, base::Value(true));
}

void FydeOsHandler::HandleCleanOfflineLoginPassword(
    const base::Value::List& args) {
  CHECK_EQ(1u, args.size());
  const base::Value& callback_id = args[0];
  if (!g_browser_process) {
    ResolveJavascriptCallback(callback_id, base::Value(false));
    return;
  }
  PrefService* prefs = g_browser_process->local_state();
  prefs->SetString(fydeos::prefs::kOfflineAutoSigninPassword, std::string());
  prefs->SetString(fydeos::prefs::kOfflineAutoSigninAccountIdKey,
      std::string());
  ResolveJavascriptCallback(callback_id, base::Value(true));
}

void FydeOsHandler::HandleSetShowRotateScreenButton(
    const base::Value::List& args) {
  CHECK_EQ(1u, args.size());
  bool showRotate = args[0].GetBool();
  PrefService* prefs = g_browser_process->local_state();
  prefs->SetBoolean(fydeos::prefs::kShowRotateScreenButton, showRotate);
}

void FydeOsHandler::HandleGetShowRotateScreenButton(
    const base::Value::List& args) {
  AllowJavascript();

  DCHECK(args.size());
  std::string callback_id = args[0].GetString();
  PrefService* prefs = g_browser_process->local_state();
  bool showRotate = prefs->GetBoolean(fydeos::prefs::kShowRotateScreenButton);
  ResolveJavascriptCallback(base::Value(callback_id),
                            base::Value(showRotate));
}

void FydeOsHandler::OnTabletPhysicalStateChanged() {
  if (!IsJavascriptAllowed()) {
    return;
  }
  bool is_in_tablet_physical_state =
    ash::Shell::Get()->tablet_mode_controller()->is_in_tablet_physical_state();
  FireWebUIListener("is-in-tablet-physical-state-changed",
                    base::Value(is_in_tablet_physical_state));
}

void FydeOsHandler::HandleGetIsInTabletPhysicalState(
    const base::Value::List& args) {
  AllowJavascript();

  DCHECK(args.size());
  std::string callback_id = args[0].GetString();
  bool is_in_tablet_physical_state =
    ash::Shell::Get()->tablet_mode_controller()->is_in_tablet_physical_state();
  ResolveJavascriptCallback(base::Value(callback_id),
                            base::Value(is_in_tablet_physical_state));
}

void FydeOsHandler::OnShowSwitchTabletLaptopButtonChanged() {
  PrefService* prefs = g_browser_process->local_state();
  bool showButton = prefs->GetBoolean(fydeos::prefs::kShowSwitchTabletLaptopButton);
  FireWebUIListener("show-switch-tablet-laptop-button-changed",
                    base::Value(showButton));
}

void FydeOsHandler::HandleSetShowSwitchTabletLaptopButton(const base::Value::List& args) {
  CHECK_EQ(1u, args.size());
  bool showButton = args[0].GetBool();
  PrefService* prefs = g_browser_process->local_state();
  prefs->SetBoolean(fydeos::prefs::kShowSwitchTabletLaptopButton, showButton);
}

void FydeOsHandler::HandleGetShowSwitchTabletLaptopButton(const base::Value::List& args) {
  AllowJavascript();

  DCHECK(args.size());
  std::string callback_id = args[0].GetString();
  PrefService* prefs = g_browser_process->local_state();
  bool showButton = prefs->GetBoolean(fydeos::prefs::kShowSwitchTabletLaptopButton);
  ResolveJavascriptCallback(base::Value(callback_id),
                            base::Value(showButton));
}

void FydeOsHandler::HandleGetIsForceTpmFallback(const base::Value::List& args) {
  AllowJavascript();

  DCHECK(args.size());
  std::string callback_id = args[0].GetString();
  PrefService* prefs = g_browser_process->local_state();
  base::Value::Dict response;
  bool tpm_fallback = prefs->GetBoolean(fydeos::prefs::kForceTpmFallback);
  bool current_tpm_fallback =
    prefs->GetBoolean(fydeos::prefs::kCurrentForceTpmFallback);
  response.Set("current", current_tpm_fallback);
  response.Set("pref", tpm_fallback);
  ResolveJavascriptCallback(base::Value(callback_id), response);
}

void FydeOsHandler::HandleSetForceTpmFallback(const base::Value::List& args) {
  CHECK_EQ(1u, args.size());
  bool tpm_fallback = args[0].GetBool();
  if (!user_manager::UserManager::Get()->IsCurrentUserOwner()) {
    VLOG(2) << "non owner user tried to set "
      << fydeos::prefs::kForceTpmFallback << ", ignored";
    return;
  }
  PrefService* prefs = g_browser_process->local_state();
  const bool necessary =
    prefs->GetBoolean(fydeos::prefs::kForceTpmFallbackNecessary);
  if (!necessary) {
    VLOG(2) << "tried to set " << fydeos::prefs::kForceTpmFallback
      << ", which is not necessary, ignored";
    return;
  }
  prefs->SetBoolean(fydeos::prefs::kForceTpmFallback, tpm_fallback);
}

void FydeOsHandler::OnForceTpmFallbackChanged() {
  PrefService* prefs = g_browser_process->local_state();
  base::Value::Dict response;
  bool tpm_fallback = prefs->GetBoolean(fydeos::prefs::kForceTpmFallback);
  bool current_tpm_fallback =
    prefs->GetBoolean(fydeos::prefs::kCurrentForceTpmFallback);
  response.Set("current", current_tpm_fallback);
  response.Set("pref", tpm_fallback);
  FireWebUIListener("force-tpm-fallback-changed", response);
}

}  // namespace ash::settings
