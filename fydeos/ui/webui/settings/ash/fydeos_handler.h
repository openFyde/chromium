// Copyright (c) 2021 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_SETTINGS_CHROMEOS_FYDEOS_HANDLER_H_
#define CHROME_BROWSER_UI_WEBUI_SETTINGS_CHROMEOS_FYDEOS_HANDLER_H_

#include "base/memory/weak_ptr.h"
#include "chrome/browser/ui/webui/settings/settings_page_ui_handler.h"
#include "components/prefs/pref_change_registrar.h"
#include "ash/public/cpp/tablet_mode_observer.h"

class PrefService;
class Profile;

namespace base {
class ListValue;
}

namespace ash::settings {

class FydeOsHandler :
    public ::settings::SettingsPageUIHandler,
    public ash::TabletModeObserver {
 public:
  // explicit FydeOsHandler(Profile* profile, PrefService* pref_service);
  explicit FydeOsHandler(PrefService* pref_service);
  ~FydeOsHandler() override;

  // SettingsPageUIHandler implementation.
  void RegisterMessages() override;
  void OnJavascriptAllowed() override;
  void OnJavascriptDisallowed() override;

  // TabletModeObserver:
  void OnTabletPhysicalStateChanged() override;
 private:
  void OnShowRotateScreenButtonChanged();
  void HandleSetShowRotateScreenButton(const base::Value::List& args);
  void HandleGetShowRotateScreenButton(const base::Value::List& args);
  void HandleGetIsInTabletPhysicalState(const base::Value::List& args);

  void HandleGetIsForceTpmFallback(const base::Value::List& args);
  void HandleSetForceTpmFallback(const base::Value::List& args);
  void OnForceTpmFallbackChanged();

  // Profile* profile_;
  PrefService* const prefs_;

  PrefChangeRegistrar pref_change_registrar_;
  PrefChangeRegistrar local_state_pref_change_registrar_;

  base::WeakPtrFactory<FydeOsHandler> weak_ptr_factory_{this};
};

}  // namespace ash::settings

#endif
