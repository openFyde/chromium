// Copyright (c) 2021 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fydeos/ui/webui/settings/ash/fydeos_handler.h"

#include "base/logging.h"
#include "base/values.h"
#include "base/files/file_util.h"
#include "base/task/thread_pool.h"
#include "ash/shell.h"
#include "chrome/browser/lifetime/application_lifetime.h"
#include "chrome/browser/profiles/profile.h"
#include "components/prefs/pref_service.h"
#include "chrome/browser/browser_process.h"
#include "components/user_manager/user_manager.h"
#include "ash/wm/tablet_mode/tablet_mode_controller.h"
#include "fydeos/prefs/fydeos_pref_names.h"
#include "chrome/browser/ash/profiles/profile_helper.h"
#include "ui/base/l10n/l10n_util.h"
// #include "chromeos/cryptohome/system_salt_getter.h"
// #include "chrome/browser/ash/settings/token_encryptor.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/browser_window.h"
#include "chrome/browser/ui/chrome_select_file_policy.h"
#include "content/public/browser/browser_thread.h"
#include "chrome/browser/ash/file_manager/path_util.h"
#include "chrome/grit/generated_resources.h"
#include "chrome/browser/ash/file_manager/volume_manager.h"
#include "fydeos/switches/misc/misc_constants.h"

namespace ash::settings {

namespace {

bool SuitableForBackupVolume(const file_manager::Volume* volume) {
  if (!volume) {
    return false;
  }
  return volume->type() == file_manager::VOLUME_TYPE_REMOVABLE_DISK_PARTITION
    && base::StartsWith(volume->mount_path().value(),
        "/media/removable", base::CompareCase::INSENSITIVE_ASCII)
    && !volume->hidden()
    && !volume->is_read_only_removable_device()
    && !volume->is_read_only()
    && volume->has_media();
}

}  // namespace

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
  if (select_file_dialog_.get())
    select_file_dialog_->ListenerDestroyed();
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
      "getShowRebootButtonInTray",
      base::BindRepeating(&FydeOsHandler::HandleGetShowRebootButtonInTray,
                          base::Unretained(this)));

  web_ui()->RegisterMessageCallback(
      "setShowRebootButtonInTray",
      base::BindRepeating(&FydeOsHandler::HandleSetShowRebootButtonInTray,
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

  web_ui()->RegisterMessageCallback(
      "selectLibwidevineFile",
      base::BindRepeating(&FydeOsHandler::HandleSelectLibwidevineFile,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "getRebootRequiredForWidevine",
      base::BindRepeating(&FydeOsHandler::HandleGetRebootRequiredForWidevine,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "toggleRebootRequiredForWidevine",
      base::BindRepeating(&FydeOsHandler::HandleToggleRebootRequiredForWidevine,
                          base::Unretained(this)));

  web_ui()->RegisterMessageCallback(
      "fydeosBackupSupported",
      base::BindRepeating(&FydeOsHandler::HandleFydeOSBackupSupported,
                          base::Unretained(this)));

  web_ui()->RegisterMessageCallback(
      "fydeosBackupSelectFile",
      base::BindRepeating(&FydeOsHandler::HandleFydeOSBackupSelectFile,
                          base::Unretained(this)));

  web_ui()->RegisterMessageCallback(
      "fydeosBackupStarted",
      base::BindRepeating(&FydeOsHandler::HandleFydeOSBackupStarted,
                          base::Unretained(this)));

  web_ui()->RegisterMessageCallback(
      "getFydeosBackupState",
      base::BindRepeating(&FydeOsHandler::HandleGetFydeOSBackupState,
                          base::Unretained(this)));
}

void FydeOsHandler::OnJavascriptAllowed() {
  pref_change_registrar_.Init(prefs_);

  local_state_pref_change_registrar_.Init(g_browser_process->local_state());
  local_state_pref_change_registrar_.Add(
      fydeos::prefs::kShowRebootButtonInTray,
      base::BindRepeating(
          &FydeOsHandler::OnShowRebootButtonInTrayChanged,
          base::Unretained(this)));
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
  base::Value response(base::Value::Type::DICT);
  response.SetBoolKey("enabled", !account_id_key.empty() && !password.empty());
  const user_manager::User* user =
      ProfileHelper::Get()->GetUserByProfile(profile_);
  if (!user->GetAccountId().HasAccountIdKey() || !user->IsFlintAccountUser()) {
    response.SetBoolKey("is_current_user", false);
  }
  if (user->GetAccountId().GetAccountIdKey() == account_id_key) {
    response.SetBoolKey("is_current_user", true);
  } else {
    response.SetBoolKey("is_current_user", false);
  }
  response.SetBoolKey("system_salt_obtained", !system_salt_.empty());
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

void FydeOsHandler::OnShowRebootButtonInTrayChanged() {
  PrefService* prefs = g_browser_process->local_state();
  bool showReboot = prefs->GetBoolean(fydeos::prefs::kShowRebootButtonInTray);
  FireWebUIListener("show-reboot-button-in-tray-changed",
                    base::Value(showReboot));
}

void FydeOsHandler::HandleSetShowRebootButtonInTray(
    const base::Value::List& args) {
  CHECK_EQ(1u, args.size());
  bool showReboot = args[0].GetBool();
  PrefService* prefs = g_browser_process->local_state();
  prefs->SetBoolean(fydeos::prefs::kShowRebootButtonInTray, showReboot);
}

void FydeOsHandler::HandleGetShowRebootButtonInTray(
    const base::Value::List& args) {
  AllowJavascript();

  DCHECK(args.size());
  std::string callback_id = args[0].GetString();
  PrefService* prefs = g_browser_process->local_state();
  bool showReboot = prefs->GetBoolean(fydeos::prefs::kShowRebootButtonInTray);
  ResolveJavascriptCallback(base::Value(callback_id),
                            base::Value(showReboot));
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
  base::Value response(base::Value::Type::DICT);
  bool tpm_fallback = prefs->GetBoolean(fydeos::prefs::kForceTpmFallback);
  bool current_tpm_fallback =
    prefs->GetBoolean(fydeos::prefs::kCurrentForceTpmFallback);
  response.SetBoolKey("current", current_tpm_fallback);
  response.SetBoolKey("pref", tpm_fallback);
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
  base::Value response(base::Value::Type::DICT);
  bool tpm_fallback = prefs->GetBoolean(fydeos::prefs::kForceTpmFallback);
  bool current_tpm_fallback =
    prefs->GetBoolean(fydeos::prefs::kCurrentForceTpmFallback);
  response.SetBoolKey("current", current_tpm_fallback);
  response.SetBoolKey("pref", tpm_fallback);
  FireWebUIListener("force-tpm-fallback-changed", response);
}

void FydeOsHandler::HandleSelectLibwidevineFile(const base::Value::List& args) {
  CHECK_EQ(0u, args.size());
  select_file_dialog_ = ui::SelectFileDialog::Create(
      this,
      std::make_unique<ChromeSelectFilePolicy>(web_ui()->GetWebContents()));
  ui::SelectFileDialog::FileTypeInfo file_type_info;
  file_type_info.allowed_paths =
    ui::SelectFileDialog::FileTypeInfo::NATIVE_PATH;
  file_type_info.extensions.resize(1);
  file_type_info.extensions[0].push_back(FILE_PATH_LITERAL("so"));
  Browser* browser =
      chrome::FindBrowserWithWebContents(web_ui()->GetWebContents());
  base::FilePath default_path =
    file_manager::util::GetDownloadsFolderForProfile(profile_);
  file_dialog_type_ = FileDialogType::kLibwidevine;
  select_file_dialog_->SelectFile(
      ui::SelectFileDialog::SELECT_OPEN_FILE,
      l10n_util::GetStringUTF16(
        IDS_OS_SETTINGS_FYDEOS_SELECT_WIDEVINE_FILE_DIALOG_TITLE),
      default_path, &file_type_info, 0, base::FilePath::StringType(),
      browser->window()->GetNativeWindow(), nullptr);
}

void FydeOsHandler::HandleGetRebootRequiredForWidevine(
    const base::Value::List& args) {
  DCHECK(args.size());
  std::string callback_id = args[0].GetString();
  PrefService* prefs = g_browser_process->local_state();
  bool rebootRequired = prefs->GetBoolean(
      fydeos::prefs::kRebootRequiredForWidevine);
  ResolveJavascriptCallback(callback_id, base::Value(rebootRequired));
}

void FydeOsHandler::HandleToggleRebootRequiredForWidevine(
    const base::Value::List& args) {
  if (args.size() < 2 || !args[0].is_string() || !args[1].is_bool()) {
    NOTREACHED();
    return;
  }
  std::string callback_id = args[0].GetString();
  bool force = args[1].GetBool();
  PrefService* prefs = g_browser_process->local_state();
  bool current = prefs->GetBoolean(fydeos::prefs::kRebootRequiredForWidevine);
  bool rebootRequired;
  if (force) {
    nextToggleRebootRequiredForWidevine_ = current;
    rebootRequired = true;
    lastToggleRebootRequiredForce_ = true;
  } else {
    if (lastToggleRebootRequiredForce_) {
      rebootRequired = nextToggleRebootRequiredForWidevine_;
    } else {
      rebootRequired = !current;
    }
    lastToggleRebootRequiredForce_ = false;
  }
  prefs->SetBoolean(fydeos::prefs::kRebootRequiredForWidevine, rebootRequired);
  ResolveJavascriptCallback(callback_id, base::Value(rebootRequired));
}

void FydeOsHandler::FileSelected(const base::FilePath& path,
                                 int /*index*/,
                                 void* /*params*/) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

  select_file_dialog_ = nullptr;
  switch (file_dialog_type_) {
    case FileDialogType::kLibwidevine:
      OnLibwidevineFileSelected(path);
      break;
    case FileDialogType::kBackup:
      OnBackupFileSelected(path);
      break;
    case FileDialogType::kUnspecified:
      NOTREACHED();
      break;
  }
}

void FydeOsHandler::FileSelectionCanceled(void* params) {
  select_file_dialog_ = nullptr;
  switch (file_dialog_type_) {
    case FileDialogType::kLibwidevine:
      OnLibwidevineFileSelectionCanceled();
      break;
    case FileDialogType::kBackup:
      OnBackupFileSelectionCanceled();
      break;
    case FileDialogType::kUnspecified:
      NOTREACHED();
      break;
  }
}

void FydeOsHandler::OnLibwidevineFileSelected(const base::FilePath& path) {
  FireWebUIListener("fydeos-libwidevine-file-selected",
      base::Value(path.value()));
}

void FydeOsHandler::OnLibwidevineFileSelectionCanceled() {
  FireWebUIListener("fydeos-libwidevine-file-selected", base::Value());
}

void FydeOsHandler::HandleFydeOSBackupSupported(
    const base::Value::List& args) {
  AllowJavascript();
  CHECK_EQ(1u, args.size());
  const std::string& callback_id = args[0].GetString();
  base::ThreadPool::PostTaskAndReplyWithResult(
      FROM_HERE, {base::MayBlock(), base::TaskPriority::USER_VISIBLE},
      base::BindOnce(
        &base::PathExists,
        base::FilePath(fydeos::constants::kFydeOSBackupScriptPath)),
      base::BindOnce(&FydeOsHandler::OnFydeOSBackupScriptChecked,
                     weak_ptr_factory_.GetWeakPtr(), callback_id));
}

void FydeOsHandler::OnFydeOSBackupScriptChecked(const std::string& callback_id,
                                                bool is_backup_supported) {
  ResolveJavascriptCallback(
      base::Value(callback_id), base::Value(is_backup_supported));
}

void FydeOsHandler::HandleFydeOSBackupSelectFile(
    const base::Value::List& args) {
  DCHECK(args.size());
  std::string default_filename = args[0].GetString();
  select_file_dialog_ = ui::SelectFileDialog::Create(
      this,
      std::make_unique<ChromeSelectFilePolicy>(web_ui()->GetWebContents()));

  ui::SelectFileDialog::FileTypeInfo file_type_info;
  file_type_info.allowed_paths =
    ui::SelectFileDialog::FileTypeInfo::NATIVE_PATH;
  file_type_info.extensions.resize(1);
  file_type_info.extensions[0].push_back(FILE_PATH_LITERAL(".bak"));

  Browser* browser =
      chrome::FindBrowserWithWebContents(web_ui()->GetWebContents());

  auto* volume_manager = file_manager::VolumeManager::Get(profile_);
  file_manager::Volume* volume = nullptr;
  for (auto& v : volume_manager->GetVolumeList()) {
    if (SuitableForBackupVolume(v.get())) {
      volume = v.get();
      break;
    }
  }
  base::FilePath default_path = volume
    ? volume->mount_path()
    : file_manager::util::GetMyFilesFolderForProfile(profile_);
  const base::FilePath default_filepath = default_path.Append(default_filename);
  file_dialog_type_ = FileDialogType::kBackup;
  select_file_dialog_->SelectFile(
      ui::SelectFileDialog::SELECT_SAVEAS_FILE,
      l10n_util::GetStringUTF16(
        IDS_OS_SETTINGS_FYDEOS_BACKUP_SAVE_FILE_DIALOG_TITLE),
      default_filepath, &file_type_info, 0, base::FilePath::StringType(),
      browser->window()->GetNativeWindow(), nullptr);
}

void FydeOsHandler::OnBackupFileSelected(const base::FilePath& path) {
  BackupTaskManager::GetInstance()->SetBackupFile(path);
  const bool canceled = false;
  FireWebUIListener("fydeos-backup-file-selected", base::Value(canceled));
}

void FydeOsHandler::OnBackupFileSelectionCanceled() {
  const bool canceled = true;
  FireWebUIListener("fydeos-backup-file-selected", base::Value(canceled));
}

void FydeOsHandler::HandleFydeOSBackupStarted(const base::Value::List& args) {
  DCHECK_EQ(args.size(), 2u);
  std::string email = args[0].GetString();
  std::string password = args[1].GetString();

  BackupTaskManager::GetInstance()->SetCallback(
      base::BindRepeating(&FydeOsHandler::OnBackupTaskFinished,
                          weak_ptr_factory_.GetWeakPtr()));
  BackupTaskManager::GetInstance()->StartTask(profile_, email, password);
}

void FydeOsHandler::OnBackupTaskFinished(BackupTaskManager::TaskState state) {
  if (state == BackupTaskManager::TaskState::kRunning
      || state == BackupTaskManager::TaskState::kIdle) {
    return;
  }
  FireWebUIListener("fydeos-backup-task-finished",
      base::Value(state == BackupTaskManager::TaskState::kFinished));
}

void FydeOsHandler::HandleGetFydeOSBackupState(const base::Value::List& args) {
  AllowJavascript();
  CHECK_EQ(1u, args.size());
  const base::Value& callback_id = args[0];
  BackupTaskManager::TaskState state =
    BackupTaskManager::GetInstance()->GetTaskState();
  std::string state_str;
  switch (state) {
    case BackupTaskManager::TaskState::kIdle:
    case BackupTaskManager::TaskState::kFinished:
    case BackupTaskManager::TaskState::kFailed:
      state_str = "not_running";
      break;
    case BackupTaskManager::TaskState::kRunning:
      state_str = "running";
      break;
    default:
      NOTREACHED();
      break;
  }
  ResolveJavascriptCallback(callback_id, base::Value(state_str));
}

}  // namespace ash::settings
