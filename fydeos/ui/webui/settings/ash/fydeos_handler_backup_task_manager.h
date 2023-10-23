// Copyright (c) 2023 Fyde Innovations. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef _FYDEOS_UI_WEBUI_SETTINGS_ASH_FYDEOS_HANDLER_BACKUP_TASK_MANAGER_H_
#define _FYDEOS_UI_WEBUI_SETTINGS_ASH_FYDEOS_HANDLER_BACKUP_TASK_MANAGER_H_

#include <string>
#include <utility>
#include <memory>
#include "base/memory/weak_ptr.h"
#include "third_party/abseil-cpp/absl/types/optional.h"
#include "base/functional/callback.h"
#include "fydeos/chromeos/ash/components/dbus/fydeos_shell_client/shell_state.h"
#include "base/files/file_path.h"

namespace base {
class OneShotTimer;
class FilePath;
}
namespace message_center {
class Notification;
}

class Profile;

namespace fydeos::ash {
class FydeOSShellClient;
}

namespace ash::settings {

using fydeos::ash::FydeOSShellClient;
using fydeos::ash::ShellState;

class BackupTaskManager {
 public:
    enum class TaskState {
      kIdle,
      kRunning,
      kFinished,
      kFailed,
    };
    using BackupTaskCallback = base::OnceCallback<void(TaskState)>;
    BackupTaskManager();
    ~BackupTaskManager();

    static BackupTaskManager* GetInstance();
    static void DestroyInstance();

    void StartTask(Profile* profile,
                   const std::string& email,
                   const std::string& password);
    void OnTaskStarted(absl::optional<ShellState> state);
    void GetTaskOutputAndState();
    void OnGetTaskOutputAndState(absl::optional<ShellState> state);

    void ScheduleGetTaskOutputAndState();

    void SetBackupFile(const base::FilePath& path) { backup_path_ = path; }
    TaskState GetTaskState() const { return task_state_; }
    void SetCallback(BackupTaskCallback callback) {
      callback_ = std::move(callback);
    }

 private:
    void DisplayNotification(
        std::unique_ptr<message_center::Notification> notification);

    // Handle backup finished
    void OnBackupFinished();
    void OnCheckBackupFile(const bool);
    void ShowBackupFinishedNotification(
        const base::FilePath& file_path, const bool exists);

    void GetShellClientTaskState();
    void OnGetShellClientTaskState(absl::optional<ShellState> state);

    // Handle backup error
    using ReadTmpLogCallback = base::OnceCallback<
      void(const base::FilePath& file_path, const bool exists)>;
    void OnBackupError();
    void OnCheckTmpLogFile(const bool);
    void ProcessTmpLogFileByShellClient(
        const base::FilePath& tmp, ReadTmpLogCallback callback);
    void OnTmpLogFileRead(
        ReadTmpLogCallback callback, absl::optional<ShellState> state);
    void OnLogFileCopied(ReadTmpLogCallback callback,
                         const base::FilePath& log, const bool exists);
    void ShowBackupErrorNotification(
        const base::FilePath& log, const bool exists);

    FydeOSShellClient* shell_client_ = nullptr;
    int32_t task_id_ = 0;
    TaskState task_state_ = TaskState::kIdle;
    Profile* profile_ = nullptr;
    base::FilePath backup_path_;
    base::FilePath tmp_log_path_;
    BackupTaskCallback callback_;

    scoped_refptr<base::SequencedTaskRunner> task_runner_;
    base::WeakPtrFactory<BackupTaskManager> weak_ptr_factory_{this};
};

}  // namespace ash::settings

#endif  // _FYDEOS_UI_WEBUI_SETTINGS_ASH_FYDEOS_HANDLER_BACKUP_TASK_MANAGER_H_
