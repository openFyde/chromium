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

namespace ash::settings {

// FydeOsHandler::FydeOsHandler(Profile* profile, PrefService* prefs) :
//   profile_(profile), prefs_(prefs) {}
FydeOsHandler::FydeOsHandler(PrefService* prefs) : prefs_(prefs) {
  DCHECK(ash::Shell::Get());
  ash::Shell::Get()->tablet_mode_controller()->AddObserver(this);
}

FydeOsHandler::~FydeOsHandler() {
  if (ash::Shell::Get()->tablet_mode_controller())
    ash::Shell::Get()->tablet_mode_controller()->RemoveObserver(this);
}

void FydeOsHandler::RegisterMessages() {
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
