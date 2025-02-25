// Copyright (c) 2023 Fyde Innovations. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "fydeos/ui/webui/settings/ash/fydeos_handler_backup_task_manager.h"
#include "base/notreached.h"
#include "fydeos/chromeos/ash/components/dbus/fydeos_shell_client/fydeos_shell_client.h"
#include "base/logging.h"
#include "base/time/time.h"
#include "base/task/sequenced_task_runner.h"
#include "base/strings/stringprintf.h"
#include "base/base64.h"
#include "ui/message_center/message_center.h"
#include "ui/message_center/public/cpp/notification.h"
#include "ash/public/cpp/notification_utils.h"
#include "chrome/browser/notifications/notification_handler.h"
#include "chrome/browser/notifications/notification_display_service_factory.h"
#include "chrome/browser/notifications/notification_display_service.h"
#include "components/vector_icons/vector_icons.h"
#include "base/files/file_util.h"
#include "chrome/browser/platform_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/json/json_reader.h"
#include "base/strings/string_util.h"
#include "chrome/grit/generated_resources.h"
#include "ui/base/l10n/l10n_util.h"
#include "base/hash/sha1.h"
#include "base/strings/string_number_conversions.h"
#include "ash/constants/notifier_catalogs.h"
#include "base/task/thread_pool.h"

using message_center::MessageCenter;
using message_center::Notification;

namespace ash::settings {

namespace {
  const char kFydeOSBackupCommandFormat[] =
    "/usr/bin/fydeos-backup backup --email %s --key %s --target %s";
  constexpr int kFydeOSBackupTaskTrackIntervalSeconds = 5;
  constexpr int kFydeOSBackupTaskOutputLines = 10;

  FydeOSShellClient* GetShellClient() {
    return FydeOSShellClient::Get();
  }
  BackupTaskManager* g_backup_task_manager = nullptr;

  const char kFydeOSBackupNotificationId[] = "fydeos.os-settings.backup";
  const char kFydeOSBackupNotifierId[] = "fydeos.os-settings.backup";
  std::unique_ptr<Notification> CreateNotification(
      BackupTaskManager::TaskState state,
      const std::u16string& title,
      const std::u16string& message,
      scoped_refptr<message_center::NotificationDelegate> delegate) {
    message_center::NotificationType type =
      message_center::NOTIFICATION_TYPE_SIMPLE;
    message_center::SystemNotificationWarningLevel warning_level =
      message_center::SystemNotificationWarningLevel::NORMAL;
    if (state == BackupTaskManager::TaskState::kRunning) {
      type = message_center::NotificationType::NOTIFICATION_TYPE_PROGRESS;
    }
    if (state == BackupTaskManager::TaskState::kFailed) {
      warning_level = message_center::SystemNotificationWarningLevel::WARNING;
    }
    std::unique_ptr<Notification> notification =
      ::ash::CreateSystemNotificationPtr(
        type, kFydeOSBackupNotificationId,
        title,
        message,
        std::u16string(), GURL(),
        message_center::NotifierId(
          message_center::NotifierType::SYSTEM_COMPONENT,
          kFydeOSBackupNotifierId,
          ash::NotificationCatalogName::kFydeOSDataBackup),
        message_center::RichNotificationData(),
        nullptr,
        vector_icons::kProductIcon,
        warning_level);
    if (state == BackupTaskManager::TaskState::kRunning) {
      notification->set_progress(-1);
      notification->set_pinned(true);
      notification->set_never_timeout(true);
    } else {
      notification->set_never_timeout(false);
      notification->set_pinned(false);
      if (delegate) {
        notification->set_delegate(std::move(delegate));
      }
    }
    return notification;
  }
  const std::string GenerateKey(
      const std::string& email, const std::string& password) {
    std::string key = email + ":" + password;
    std::string hex_encoded_hash = base::HexEncode(
        base::SHA1Hash(base::as_byte_span(key)));
    hex_encoded_hash.resize(16);
    return base::ToLowerASCII(hex_encoded_hash);
  }

  bool WriteFile_(const base::FilePath& path, const std::string& content) {
    int ret = base::WriteFile(path, content);
    if (ret != static_cast<int>(content.size())) {
      return false;
    }
    return base::PathExists(path);
  }
}  // namespace

BackupTaskManager* BackupTaskManager::GetInstance() {
  if (!g_backup_task_manager) {
    g_backup_task_manager = new BackupTaskManager();
  }
  return g_backup_task_manager;
}

void BackupTaskManager::DestroyInstance() {
  if (g_backup_task_manager) {
    delete g_backup_task_manager;
    g_backup_task_manager = nullptr;
  }
}

BackupTaskManager::BackupTaskManager() {
  task_runner_ = base::ThreadPool::CreateSequencedTaskRunner(
      {base::MayBlock(), base::TaskPriority::BEST_EFFORT});
}

BackupTaskManager::~BackupTaskManager() {
  if (MessageCenter::Get()->FindVisibleNotificationById(
        kFydeOSBackupNotificationId)) {
    MessageCenter::Get()->RemoveNotification(
        kFydeOSBackupNotificationId, false);
  }
}

void BackupTaskManager::StartTask(Profile* profile,
                                  const std::string& email,
                                  const std::string& password) {
  profile_ = profile;
  if (!shell_client_) {
    shell_client_ = GetShellClient();
  }
  std::string encoded_filepath = base::Base64Encode(backup_path_.value());
  task_state_ = TaskState::kRunning;
  const std::string key = GenerateKey(email, password);
  const std::string command = base::StringPrintf(
      kFydeOSBackupCommandFormat,
      email.c_str(), key.c_str(), encoded_filepath.c_str());
  shell_client_->AsyncExec(command,
      base::BindOnce(
        &BackupTaskManager::OnTaskStarted, weak_ptr_factory_.GetWeakPtr()));
}

void BackupTaskManager::OnTaskStarted(std::optional<ShellState> state) {
  if (!state || state->code == -1) {
    LOG(ERROR) << "start backup task error, "
               << (state ? state->result : "state is null");
    task_state_ = TaskState::kFailed;
    return;
  }
  task_id_ = state->code;

  // for starting a new backup task,
  // force remove and add notification to make sure it will popup
  if (MessageCenter::Get()->FindVisibleNotificationById(
        kFydeOSBackupNotificationId)) {
    MessageCenter::Get()->RemoveNotification(
        kFydeOSBackupNotificationId, false);
  }
  DisplayNotification(CreateNotification(
        task_state_, l10n_util::GetStringUTF16(
          IDS_FYDEOS_BACKUP_NOTIFICATION_RUNNING_TITLE), u"", nullptr));
  GetShellClientTaskState();
  GetTaskOutputAndState();
}

void BackupTaskManager::ScheduleGetTaskOutputAndState() {
  base::SequencedTaskRunner::GetCurrentDefault()->PostDelayedTask(
      FROM_HERE,
      base::BindOnce(
        &BackupTaskManager::GetTaskOutputAndState,
        weak_ptr_factory_.GetWeakPtr()),
      base::Seconds(kFydeOSBackupTaskTrackIntervalSeconds));
}

void BackupTaskManager::GetTaskOutputAndState() {
  shell_client_->GetTaskOutput(task_id_, kFydeOSBackupTaskOutputLines,
      base::BindOnce(
        &BackupTaskManager::OnGetTaskOutputAndState,
        weak_ptr_factory_.GetWeakPtr()));
}

void BackupTaskManager::OnGetTaskOutputAndState(
    std::optional<ShellState> state) {
  if (!state || state->code == -1) {
    LOG(ERROR) << "get backup task output and state error, "
               << (state ? state->result : "state is null");
    return;
  }
  // see fydeos/extensions/common/api/shell_client.json
  switch (state->code) {
    case 0:
      // ON_NONE
      task_state_ = TaskState::kFailed;
      OnBackupError();
      break;
    case 1:
      // ON_PROGRESS
      task_state_ = TaskState::kRunning;
      ScheduleGetTaskOutputAndState();
      break;
    case 2:
      // ON_CLOSED
      task_state_ = TaskState::kFinished;
      OnBackupFinished();
      break;
    case 3:
      // ON_ERROR
      task_state_ = TaskState::kFailed;
      OnBackupError();
      break;
    default:
      NOTREACHED_IN_MIGRATION();
      break;
  }
  if (!callback_.is_null()
      && task_state_ != TaskState::kRunning
      && task_state_ != TaskState::kIdle) {
    std::move(callback_).Run(task_state_);
  }
}

void BackupTaskManager::GetShellClientTaskState() {
  shell_client_->GetTaskState(task_id_,
      base::BindOnce(
        &BackupTaskManager::OnGetShellClientTaskState,
        weak_ptr_factory_.GetWeakPtr()));
}

void BackupTaskManager::OnGetShellClientTaskState(
    std::optional<ShellState> state) {
  if (!state || state->code == -1) {
    LOG(ERROR) << "get backup task final state error, "
               << (state ? state->result : "state is null");
    return;
  }
  std::optional<base::Value::Dict> json = base::JSONReader::ReadDict(state->result);
  if (!json) {
    LOG(ERROR) << "get backup task final state error, json is invalid";
    return;
  }
  if (json->FindInt("key") == task_id_) {
    const std::string *tmpFile = json->FindString("tmpFile");
    if (tmpFile) {
      tmp_log_path_ = base::FilePath(*tmpFile);
    }
    return;
  }
}

void BackupTaskManager::OnBackupFinished() {
  task_runner_->PostTaskAndReplyWithResult(
      FROM_HERE,
      base::BindOnce(&base::PathExists, backup_path_),
      base::BindOnce(&BackupTaskManager::OnCheckBackupFile,
                     weak_ptr_factory_.GetWeakPtr()));
}

void BackupTaskManager::OnCheckBackupFile(const bool ok) {
  ShowBackupFinishedNotification(backup_path_, ok);
}

void BackupTaskManager::OnBackupError() {
  task_runner_->PostTaskAndReplyWithResult(
      FROM_HERE,
      base::BindOnce(&base::PathExists, tmp_log_path_),
      base::BindOnce(&BackupTaskManager::OnCheckTmpLogFile,
                     weak_ptr_factory_.GetWeakPtr()));
}

void BackupTaskManager::OnCheckTmpLogFile(const bool ok) {
  if (ok) {
    ProcessTmpLogFileByShellClient(tmp_log_path_, base::BindOnce(
          &BackupTaskManager::ShowBackupErrorNotification,
          weak_ptr_factory_.GetWeakPtr()));
  } else {
    VLOG(2) << "tmp log file not exist";
    ShowBackupErrorNotification(base::FilePath(), false);
  }
}

void BackupTaskManager::ProcessTmpLogFileByShellClient(
    const base::FilePath& tmp, ReadTmpLogCallback callback) {
  const std::string command = base::StringPrintf("cat %s", tmp.value().c_str());
  shell_client_->SyncExec(command,
      base::BindOnce(
        &BackupTaskManager::OnTmpLogFileRead,
        weak_ptr_factory_.GetWeakPtr(), std::move(callback)));
}

void BackupTaskManager::OnTmpLogFileRead(
    ReadTmpLogCallback callback, std::optional<ShellState> state) {
  if (!state || state->code == -1) {
    LOG(ERROR) << "read tmp log file error, "
               << (state ? state->result : "state is null");
    std::move(callback).Run(base::FilePath(), false);
    return;
  }
  const std::string name = backup_path_.value();
  base::FilePath log_path;
  if (base::EndsWith(name, ".tar.gz.gpg")) {
    log_path = base::FilePath(
        name.substr(0, name.size() - 11)).AddExtension("txt");
  } else {
    log_path = base::FilePath(name).ReplaceExtension("txt");
  }
  if (state->result.size() > 0) {
    task_runner_->PostTaskAndReplyWithResult(
        FROM_HERE,
        base::BindOnce(&WriteFile_, log_path, state->result),
        base::BindOnce(
          &BackupTaskManager::OnLogFileCopied,
          weak_ptr_factory_.GetWeakPtr(), std::move(callback), log_path));
  } else {
    std::move(callback).Run(base::FilePath(), false);
  }
}

void BackupTaskManager::OnLogFileCopied(
    ReadTmpLogCallback callback, const base::FilePath& log, const bool exists) {
  if (!exists) {
    VLOG(2) << "tmp log file copy failed";
  }
  std::move(callback).Run(log, exists);
}

void BackupTaskManager::ShowBackupErrorNotification(
    const base::FilePath& log_path, const bool exists) {
  scoped_refptr<message_center::NotificationDelegate> delegate;
  std::u16string title = l10n_util::GetStringUTF16(
      IDS_FYDEOS_BACKUP_NOTIFICATION_FAILED_TITLE);
  std::u16string message = l10n_util::GetStringUTF16(
      IDS_FYDEOS_BACKUP_NOTIFICATION_FAILED_MESSAGE);
  if (exists) {
    delegate = base::MakeRefCounted<
      message_center::HandleNotificationClickDelegate>(
        base::BindRepeating([](Profile* profile, base::FilePath path) {
          platform_util::ShowItemInFolder(profile, path);
        },
        profile_, log_path));
  } else {
    delegate = nullptr;
    message = u"";
  }
  DisplayNotification(CreateNotification(TaskState::kFailed,
        title, message, delegate));
}

void BackupTaskManager::ShowBackupFinishedNotification(
    const base::FilePath& backup_path, const bool exists) {
  scoped_refptr<message_center::NotificationDelegate> delegate;
  std::u16string title = l10n_util::GetStringUTF16(
      IDS_FYDEOS_BACKUP_NOTIFICATION_FINISHED_TITLE);
  std::u16string message = l10n_util::GetStringUTF16(
      IDS_FYDEOS_BACKUP_NOTIFICATION_FINISHED_MESSAGE);
  if (exists) {
    delegate = base::MakeRefCounted<
      message_center::HandleNotificationClickDelegate>(
        base::BindRepeating([](Profile* profile, base::FilePath path) {
          platform_util::ShowItemInFolder(profile, path);
        },
        profile_, backup_path));
  } else {
    message = u"";
    delegate = nullptr;
  }
  DisplayNotification(CreateNotification(TaskState::kFinished,
        title, message, delegate));
}

void BackupTaskManager::DisplayNotification(
    std::unique_ptr<message_center::Notification> notification) {
  if (MessageCenter::Get()->FindVisibleNotificationById(
        kFydeOSBackupNotificationId)) {
    MessageCenter::Get()->UpdateNotification(
        kFydeOSBackupNotificationId, std::move(notification));
  } else {
    MessageCenter::Get()->AddNotification(std::move(notification));
  }
}

}  // namespace ash::settings

