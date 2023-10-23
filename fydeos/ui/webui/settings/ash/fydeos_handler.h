// Copyright (c) 2021 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_SETTINGS_CHROMEOS_FYDEOS_HANDLER_H_
#define CHROME_BROWSER_UI_WEBUI_SETTINGS_CHROMEOS_FYDEOS_HANDLER_H_

#include "base/memory/weak_ptr.h"
#include "chrome/browser/ui/webui/settings/settings_page_ui_handler.h"
#include "components/prefs/pref_change_registrar.h"
#include "ash/public/cpp/tablet_mode_observer.h"
#include "ui/shell_dialogs/select_file_dialog.h"
#include "fydeos/ui/webui/settings/ash/fydeos_handler_backup_task_manager.h"

class PrefService;
class Profile;

namespace base {
class ListValue;
class FilePath;
}

namespace ash::settings {

class FydeOsHandler :
    public ::settings::SettingsPageUIHandler,
    public ui::SelectFileDialog::Listener,
    public ash::TabletModeObserver {
 public:
  explicit FydeOsHandler(Profile* profile, PrefService* pref_service);
  ~FydeOsHandler() override;

  // SettingsPageUIHandler implementation.
  void RegisterMessages() override;
  void OnJavascriptAllowed() override;
  void OnJavascriptDisallowed() override;

  // TabletModeObserver:
  void OnTabletPhysicalStateChanged() override;
 private:
  enum class FileDialogType {
    kUnspecified,
    kLibwidevine,
    kBackup,
  };
  void OnSystemSaltObtained(const std::string& system_salt);
  void HandleGetIsOfflineAutoSigninEnabled(const base::Value::List& args);
  void HandleSaveOfflineLoginPassword(const base::Value::List& args);
  void HandleCleanOfflineLoginPassword(const base::Value::List& args);

  void OnShowRebootButtonInTrayChanged();
  void HandleSetShowRebootButtonInTray(const base::Value::List& args);
  void HandleGetShowRebootButtonInTray(const base::Value::List& args);

  void OnShowRotateScreenButtonChanged();
  void HandleSetShowRotateScreenButton(const base::Value::List& args);
  void HandleGetShowRotateScreenButton(const base::Value::List& args);
  void HandleGetIsInTabletPhysicalState(const base::Value::List& args);

  void OnShowSwitchTabletLaptopButtonChanged();
  void HandleSetShowSwitchTabletLaptopButton(const base::Value::List& args);
  void HandleGetShowSwitchTabletLaptopButton(const base::Value::List& args);

  void HandleGetIsForceTpmFallback(const base::Value::List& args);
  void HandleSetForceTpmFallback(const base::Value::List& args);
  void OnForceTpmFallbackChanged();

  void HandleSelectLibwidevineFile(const base::Value::List& args);
  void HandleGetRebootRequiredForWidevine(const base::Value::List& args);
  void HandleToggleRebootRequiredForWidevine(const base::Value::List& args);


  void FileSelected(
      const base::FilePath& path, int index, void *params) override;
  void FileSelectionCanceled(void* params) override;

  void OnLibwidevineFileSelected(const base::FilePath& path);
  void OnLibwidevineFileSelectionCanceled();

  bool nextToggleRebootRequiredForWidevine_ = false;
  bool lastToggleRebootRequiredForce_ = false;

  void HandleFydeOSBackupSupported(const base::Value::List& args);
  void HandleFydeOSBackupSelectFile(const base::Value::List& args);
  void HandleFydeOSBackupStarted(const base::Value::List& args);
  void HandleGetFydeOSBackupState(const base::Value::List& args);

  void OnFydeOSBackupScriptChecked(const std::string& callback_id,
                                   bool supported);

  void OnBackupFileSelected(const base::FilePath& path);
  void OnBackupFileSelectionCanceled();

  void OnBackupTaskFinished(BackupTaskManager::TaskState state);

  std::string system_salt_;
  Profile* profile_;
  PrefService* const prefs_;

  scoped_refptr<ui::SelectFileDialog> select_file_dialog_;
  FileDialogType file_dialog_type_ = FileDialogType::kUnspecified;

  PrefChangeRegistrar pref_change_registrar_;
  PrefChangeRegistrar local_state_pref_change_registrar_;

  base::WeakPtrFactory<FydeOsHandler> weak_ptr_factory_{this};
};

}  // namespace ash::settings

#endif
