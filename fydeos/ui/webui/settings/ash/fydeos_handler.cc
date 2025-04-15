// Copyright (c) 2021 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fydeos/ui/webui/settings/ash/fydeos_handler.h"

#include "base/logging.h"
#include "base/notreached.h"
#include "base/values.h"
#include "base/files/file_util.h"
#include "base/task/thread_pool.h"
#include "ash/shell.h"
#include "chrome/browser/lifetime/application_lifetime.h"
#include "chrome/browser/profiles/profile.h"
#include "chromeos/ash/components/cryptohome/error_util.h"
#include "chromeos/ash/components/cryptohome/userdataauth_util.h"
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
#include "ui/shell_dialogs/selected_file_info.h"
#include "chrome/browser/ash/file_manager/volume_manager.h"
#include "fydeos/switches/misc/misc_constants.h"
#include "fydeos/misc/fydeos_dev_mode.h"
#include "chromeos/ash/components/login/auth/auth_factor_editor.h"
#include "chromeos/ash/components/cryptohome/auth_factor_conversions.h"

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

const char kFydeOSArcMediaAutoScanIndicatorFile[] = "/home/chronos/user/.enable_arc_media_auto_scan";

bool ArcMediaAutoScanIndicatorFileExists() {
  return base::PathExists(base::FilePath(kFydeOSArcMediaAutoScanIndicatorFile));
}

bool DeleteArcMediaAutoScanIndicatorFile() {
  return base::DeleteFile(base::FilePath(kFydeOSArcMediaAutoScanIndicatorFile));
}

bool CreateArcMediaAutoScanIndicatorFile() {
  return base::WriteFile(base::FilePath(kFydeOSArcMediaAutoScanIndicatorFile), "");
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

  web_ui()->RegisterMessageCallback(
      "getArcMediaAutoScanState",
      base::BindRepeating(&FydeOsHandler::HandleGetArcMediaAutoScanState,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "setArcMediaAutoScanState",
      base::BindRepeating(&FydeOsHandler::HandleSetArcMediaAutoScanState,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "setArcMediaAutoScanStateForCurrentSession",
      base::BindRepeating(&FydeOsHandler::HandleSetArcMediaAutoScanStateForCurrentSession,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "setDevMode",
      base::BindRepeating(&FydeOsHandler::HandleSetDevMode,
                      base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "getDevModeSwitchSupported",
      base::BindRepeating(&FydeOsHandler::HandleGetDevModeSwitchSupported,
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

void FydeOsHandler::ListAuthFactors(const AccountId& account_id, const std::string& callback_id) {
  auto client = UserDataAuthClient::Get();
  user_data_auth::ListAuthFactorsRequest request;
  *request.mutable_account_id() =
      cryptohome::CreateAccountIdentifierFromAccountId(account_id);
  client->ListAuthFactors(
      request, base::BindOnce(&FydeOsHandler::OnListAuthFactors,
                              weak_ptr_factory_.GetWeakPtr(),
                              callback_id));
}

void FydeOsHandler::OnListAuthFactors(const std::string& callback_id, std::optional<user_data_auth::ListAuthFactorsReply> reply) {
  auto error = user_data_auth::ReplyToCryptohomeError(reply);
  if (cryptohome::HasError(error)) {
    LOG(ERROR) << "Could not list auth factors " << error;
    return;
  }
  CHECK(reply.has_value());
  auth_factor_has_password_ = false;
  for (const auto& factor_with_status_proto :
       reply->configured_auth_factors_with_status()) {
    if (factor_with_status_proto.auth_factor().type() == user_data_auth::AUTH_FACTOR_TYPE_PASSWORD) {
      auth_factor_has_password_ = true;
      break;
    }
  }

  const user_manager::User* user =
      ProfileHelper::Get()->GetUserByProfile(profile_);
  PrefService* prefs = g_browser_process->local_state();
  const std::string& password =
    prefs->GetString(fydeos::prefs::kOfflineAutoSigninPassword);
  const std::string& account_id_key =
    prefs->GetString(fydeos::prefs::kOfflineAutoSigninAccountIdKey);
  base::Value::Dict response;
  response.Set("enabled", !account_id_key.empty() && !password.empty());
  if (!user->GetAccountId().HasAccountIdKey() || !user->IsFlintAccountUser()) {
    response.Set("is_current_user", false);
  }
  if (user->GetAccountId().GetAccountIdKey() == account_id_key) {
    response.Set("is_current_user", true);
  } else {
    response.Set("is_current_user", false);
  }
  response.Set("system_salt_obtained", !system_salt_.empty());
  response.Set("auth_factor_has_password", auth_factor_has_password_);
  ResolveJavascriptCallback(callback_id, response);
}

void FydeOsHandler::HandleGetIsOfflineAutoSigninEnabled(
    const base::Value::List& args) {
  AllowJavascript();
  CHECK(args.size());
  std::string callback_id = args[0].GetString();
  const user_manager::User* user =
      ProfileHelper::Get()->GetUserByProfile(profile_);
  ListAuthFactors(user->GetAccountId(), callback_id);
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
      chrome::FindBrowserWithTab(web_ui()->GetWebContents());
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
    VLOG(2) << "Invalid arguments for toggleRebootRequiredForWidevine";
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

void FydeOsHandler::FileSelected(const ui::SelectedFileInfo& file,
                                 int /*index*/) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

  select_file_dialog_ = nullptr;
  switch (file_dialog_type_) {
    case FileDialogType::kLibwidevine:
      OnLibwidevineFileSelected(file.path());
      break;
    case FileDialogType::kBackup:
      OnBackupFileSelected(file.path());
      break;
    case FileDialogType::kUnspecified:
      NOTREACHED();
  }
}

void FydeOsHandler::FileSelectionCanceled() {
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
        base::FilePath(fydeos::constants::kFydeOSBackupScriptDirPath)),
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
      chrome::FindBrowserWithTab(web_ui()->GetWebContents());

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
      NOTREACHED_IN_MIGRATION();
      break;
  }
  ResolveJavascriptCallback(callback_id, base::Value(state_str));
}

void FydeOsHandler::HandleGetArcMediaAutoScanState(const base::Value::List& args) {
  CHECK_EQ(1u, args.size());
  const std::string& callback_id = args[0].GetString();
  base::ThreadPool::PostTaskAndReplyWithResult(
      FROM_HERE, {base::MayBlock(), base::TaskPriority::USER_VISIBLE},
      base::BindOnce(&ArcMediaAutoScanIndicatorFileExists),
      base::BindOnce(&FydeOsHandler::OnArcMediaAutoScanIndicatorFileExistenceChecked,
                    weak_ptr_factory_.GetWeakPtr(), callback_id));

}

void FydeOsHandler::OnArcMediaAutoScanIndicatorFileExistenceChecked(const std::string& callback_id, bool result) {
  const PrefService::Preference* pref = prefs_->FindPreference(fydeos::prefs::kFydeOSArcMediaAutoScanEnabled);
  int saved = 0;
  if (!pref || pref->IsDefaultValue()) {
    saved = -1;
  } else {
    const bool n = pref->GetValue()->GetBool();
    saved = n ? 1 : 0;
  }
  base::Value::Dict response;
  response.Set("enabled", result);
  response.Set("saved", saved);
  if (callback_id.empty()) {
    FireWebUIListener("fydeos-arc-media-auto-scan-changed", response);
  } else {
    ResolveJavascriptCallback(callback_id, response);
  }
}

void FydeOsHandler::HandleSetArcMediaAutoScanStateForCurrentSession(const base::Value::List& args) {
  CHECK_EQ(1u, args.size());
  bool enabled = args[0].GetBool();
  prefs_->SetBoolean(fydeos::prefs::kFydeOSArcMediaAutoScanEnabled, enabled);
}

void FydeOsHandler::HandleSetArcMediaAutoScanState(const base::Value::List& args) {
  CHECK_EQ(1u, args.size());
  bool enable = args[0].GetBool();
  if (!enable) {
    base::ThreadPool::PostTaskAndReplyWithResult(
      FROM_HERE, {base::MayBlock(), base::TaskPriority::USER_VISIBLE},
      base::BindOnce(&DeleteArcMediaAutoScanIndicatorFile),
      base::BindOnce(&FydeOsHandler::OnEnableArcMediaAutoScan,
                     weak_ptr_factory_.GetWeakPtr()));
  } else {
    const std::string empty = std::string();
    base::ThreadPool::PostTaskAndReplyWithResult(
      FROM_HERE, {base::MayBlock(), base::TaskPriority::USER_VISIBLE},
      base::BindOnce(&CreateArcMediaAutoScanIndicatorFile),
      base::BindOnce(&FydeOsHandler::OnDisableArcMediaAutoScan,
                     weak_ptr_factory_.GetWeakPtr()));
  }
}

void FydeOsHandler::OnEnableArcMediaAutoScan(bool result) {
  RefreshArcMediaAutoScanState();
}

void FydeOsHandler::OnDisableArcMediaAutoScan(bool result) {
  RefreshArcMediaAutoScanState();
}

void FydeOsHandler::RefreshArcMediaAutoScanState() {
  base::ThreadPool::PostTaskAndReplyWithResult(
      FROM_HERE, {base::MayBlock(), base::TaskPriority::USER_VISIBLE},
      base::BindOnce(&ArcMediaAutoScanIndicatorFileExists),
      base::BindOnce(&FydeOsHandler::OnArcMediaAutoScanIndicatorFileExistenceChecked,
                 weak_ptr_factory_.GetWeakPtr(), ""));
}

void FydeOsHandler::OnSetDevMode(const std::string& callback_id, bool result) {
  ResolveJavascriptCallback(base::Value(callback_id), base::Value(result));
}

void FydeOsHandler::HandleSetDevMode(const base::Value::List& args) {
  CHECK_EQ(2u, args.size());
  const std::string& callback_id = args[0].GetString();
  bool enable = args[1].GetBool();
  fydeos::misc::SetDevMode(enable, base::BindOnce(&FydeOsHandler::OnSetDevMode,
                                                  weak_ptr_factory_.GetWeakPtr(),
                                                  callback_id));
}

void FydeOsHandler::HandleGetDevModeSwitchSupported(const base::Value::List& args) {
  AllowJavascript();
  CHECK_EQ(1u, args.size());
  const std::string& callback_id = args[0].GetString();
  base::ThreadPool::PostTaskAndReplyWithResult(
      FROM_HERE, {base::MayBlock(), base::TaskPriority::USER_VISIBLE},
      base::BindOnce(&fydeos::misc::IsDevModeSwitchSupported),
      base::BindOnce(&FydeOsHandler::OnDevModeSwitchSupportedChecked,
                     weak_ptr_factory_.GetWeakPtr(), callback_id));
}

void FydeOsHandler::OnDevModeSwitchSupportedChecked(const std::string& callback_id, bool result) {
  ResolveJavascriptCallback(base::Value(callback_id), base::Value(result));
}

}  // namespace ash::settings
