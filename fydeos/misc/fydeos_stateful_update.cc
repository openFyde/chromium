#include "fydeos/misc/fydeos_stateful_update.h"

#include "base/timer/timer.h"
#include "fydeos/chromeos/ash/components/dbus/fydeos_shell_client/fydeos_shell_client.h"
#include "chromeos/ash/components/dbus/dbus_thread_manager.h"
#include "base/strings/utf_string_conversions.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "base/strings/string_number_conversions.h"
#include "ui/base/l10n/l10n_util.h"
#include "ash/strings/grit/ash_strings.h"
#include "base/strings/stringprintf.h"
#include "ui/message_center/message_center.h"
#include "ui/message_center/public/cpp/notification.h"
#include "ash/public/cpp/notification_utils.h"
#include "chrome/browser/notifications/notification_handler.h"
#include "chrome/browser/notifications/notification_display_service_factory.h"
#include "chrome/browser/notifications/notification_display_service.h"
#include "components/vector_icons/vector_icons.h"
#include "chrome/browser/lifetime/application_lifetime.h"
#include "chrome/browser/ash/crostini/crostini_features.h"
#include "ash/constants/notifier_catalogs.h"


using message_center::MessageCenter;
using message_center::Notification;

using fydeos::ash::FydeOSShellClient;
using fydeos::ash::ShellState;

namespace fydeos {

namespace misc {

namespace {
  const char kStatefulUpdateAvailableNotificationId[] =
    "fydeos.stateful.update.available";
  const char kStatefulUpdateRebootNotificationId[] =
    "fydeos.stateful.update.reboot.required";
  const char kStatefulUpdateProgressNotificationId[] =
    "fydeos.stateful.update.progress";
  const char kStatefulUpdaterNotifierId[] = "fydeos.stateful.update";

  const char kFydeOSStatefulUpdateAvailable[] = "update_available";
  const char kFydeOSStatefulDoneRebootRequired[] = "reboot";

  const int kFydeOSStatefulCheckUpdateCountMax = 3;
  const int kFydeOSStatefulCheckUpdateDelayInSeconds = 20;
  const int kFydeOSStatefulCheckStatusIntervalInSeconds = 5;

  const char kStatefulUpdateScript[] = "/usr/bin/stateful_update_wrapper.sh %s";
  const char kStatefulCheckIsUpdateAvailableParam[] = "check";
  const char kStatefulCheckUpdateStatusParam[] = "status";
  const char kStatefulDoUpdateParam[] = "";

  FydeOSShellClient* GetShellClient() {
    return FydeOSShellClient::Get();
  }
  std::string CheckUpdateCmd() {
    return base::StringPrintf(kStatefulUpdateScript,
        kStatefulCheckIsUpdateAvailableParam);
  }
  std::string DoUpdateCmd() {
    return base::StringPrintf(kStatefulUpdateScript, kStatefulDoUpdateParam);
  }
  std::string CheckUpdateStatusCmd() {
    return base::StringPrintf(kStatefulUpdateScript,
        kStatefulCheckUpdateStatusParam);
  }

  bool IsCrostiniEnabled(Profile* profile) {
    return crostini::CrostiniFeatures::Get()->IsEnabled(profile);
  }
}  // namespace

StatefulUpdater::StatefulUpdater() :
  timer_(std::make_unique<base::OneShotTimer>()),
  status_timer_(std::make_unique<base::RepeatingTimer>()),
  check_update_count_(0),
  check_status_count_(0) {}

StatefulUpdater::~StatefulUpdater() = default;

void StatefulUpdater::Start() {
  if (started_) return;
  started_ = true;
  CheckUpdateDelay();
}

void StatefulUpdater::Stop() {
  if (timer_->IsRunning()) {
    timer_->AbandonAndStop();
  }
  if (status_timer_->IsRunning()) {
    status_timer_->AbandonAndStop();
  }
  started_ = false;
}

void StatefulUpdater::CheckUpdateDelay() {
  if (!timer_->IsRunning()) {
    timer_->Start(
        FROM_HERE,
        base::Seconds(kFydeOSStatefulCheckUpdateDelayInSeconds),
        base::BindOnce(&StatefulUpdater::CheckUpdate, base::Unretained(this)));
  }
}

void StatefulUpdater::CheckUpdate() {
  if (g_browser_process->profile_manager()) {
    profile_ = g_browser_process->profile_manager()->GetActiveUserProfile();
  }
  if (!profile_) return;
  if (!IsCrostiniEnabled(profile_)) {
    VLOG(2) <<
      "crostini is not available or enabled, skip stateful update checking";
    return;
  }
  FydeOSShellClient* shellClient = GetShellClient();
  if (!shellClient) return;
  check_update_count_++;
  shellClient->SyncExec(CheckUpdateCmd(),
      base::BindOnce(
        &StatefulUpdater::OnCheckUpdateDone, base::Unretained(this)));
}

void StatefulUpdater::OnCheckUpdateDone(absl::optional<ShellState> state) {
  if (!state || state->code != 0) {
    LOG(WARNING) << "check stateful update error, "
      << (state ? state->result : "state is null");
    if (check_update_count_ < kFydeOSStatefulCheckUpdateCountMax) {
      CheckUpdateDelay();
    } else {
      LOG(ERROR) << "check stateful update failed";
    }
    return;
  }
  std::string result = state->result;
  result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
  if (result == kFydeOSStatefulUpdateAvailable) {
    GenerateUpdateNotification();
  } else if (result == kFydeOSStatefulDoneRebootRequired) {
    GenerateRebootNotification();
  } else {
    VLOG(2) << "check stateful update result: " << result;
  }
}

void StatefulUpdater::DoUpdate() {
  FydeOSShellClient* shellClient = GetShellClient();
  if (!shellClient) return;
  shellClient->AsyncExec(DoUpdateCmd(),
      base::BindOnce(
        &StatefulUpdater::OnDoUpdateStarted, base::Unretained(this)));
}

void StatefulUpdater::OnDoUpdateStarted(absl::optional<ShellState> state) {
  if (!state || state->code == -1) {
    LOG(ERROR) << "stateful update script error, "
      << (state ? state->result : "state is null");
    return;
  }

  GenerateProgressNotification();
  status_timer_->Start(
      FROM_HERE,
      base::Seconds(kFydeOSStatefulCheckStatusIntervalInSeconds),
      base::BindRepeating(
        &StatefulUpdater::CheckUpdateStatus, base::Unretained(this)));
}

void StatefulUpdater::CheckUpdateStatus() {
  if (check_status_count_ * kFydeOSStatefulCheckStatusIntervalInSeconds
      >= 1800) {
    status_timer_->AbandonAndStop();
    CloseProgressNotification();
    return;
  }

  FydeOSShellClient* shellClient = GetShellClient();
  if (!shellClient) return;
  check_status_count_++;
  shellClient->SyncExec(CheckUpdateStatusCmd(),
      base::BindOnce(
        &StatefulUpdater::OnCheckUpdateStatusDone, base::Unretained(this)));
}

void StatefulUpdater::OnCheckUpdateStatusDone(
    absl::optional<ShellState> state) {
  if (!state || state->code != 0) {
    LOG(ERROR) << "check stateful update status error, "
      << (state ? state->result : "state is null");
    status_timer_->AbandonAndStop();
    CloseProgressNotification();
  } else {
    std::string result = state->result;
    result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
    if (result == kFydeOSStatefulDoneRebootRequired) {
      status_timer_->AbandonAndStop();
      CloseProgressNotification();
      GenerateRebootNotification();
    } else {
      TrytoUpdateProgressNotification(result);
    }
  }
}

void StatefulUpdater::GenerateUpdateNotification() {
  std::unique_ptr<Notification> notification =
    ::ash::CreateSystemNotificationPtr(
      message_center::NOTIFICATION_TYPE_SIMPLE,
      kStatefulUpdateAvailableNotificationId,
      l10n_util::GetStringUTF16(
        IDS_ASH_FYDEOS_STATEFUL_UPDATE_AVAILABLE_NOTIFICATION_TITLE),
      l10n_util::GetStringUTF16(
        IDS_ASH_FYDEOS_STATEFUL_UPDATE_AVAILABLE_NOTIFICATION_MESSAGE),
      std::u16string(), GURL(),
      message_center::NotifierId(
        message_center::NotifierType::SYSTEM_COMPONENT,
        kStatefulUpdaterNotifierId,
        ::ash::NotificationCatalogName::kFydeOSMiscStatefulUpdate),
      message_center::RichNotificationData(),
      base::MakeRefCounted<message_center::HandleNotificationClickDelegate>(
        base::BindRepeating(
          &StatefulUpdater::HandleUpdateNotificationClick,
          weak_ptr_factory_.GetWeakPtr())),
      vector_icons::kBusinessIcon,
      message_center::SystemNotificationWarningLevel::NORMAL);

  notification->set_pinned(true);
  std::vector<message_center::ButtonInfo> notification_actions;
  notification_actions.emplace_back(l10n_util::GetStringUTF16(
        IDS_ASH_FYDEOS_STATEFUL_UPDATE_AVAILABLE_NOTIFICATION_ACTION_CONFIRM));

  notification->set_buttons(notification_actions);

  MessageCenter::Get()->AddNotification(std::move(notification));
}

void StatefulUpdater::CloseUpdateNotification() {
  message_center::MessageCenter::Get()
    ->RemoveNotification(kStatefulUpdateAvailableNotificationId, false);
}

void StatefulUpdater::GenerateProgressNotification() {
  if (!profile_) return;
  progress_notification_ = ::ash::CreateSystemNotificationPtr(
    message_center::NOTIFICATION_TYPE_PROGRESS,
    kStatefulUpdateProgressNotificationId,
    l10n_util::GetStringUTF16(
      IDS_ASH_FYDEOS_STATEFUL_UPDATING_PROGRESS_NOTIFICATION_TITLE),
    l10n_util::GetStringUTF16(
      IDS_ASH_FYDEOS_STATEFUL_UPDATING_PROGRESS_NOTIFICATION_MESSAGE),
    std::u16string(), GURL(),
    message_center::NotifierId(message_center::NotifierType::SYSTEM_COMPONENT,
                               kStatefulUpdaterNotifierId),
    message_center::RichNotificationData(),
    base::MakeRefCounted<message_center::HandleNotificationClickDelegate>(
      base::BindRepeating(
        &StatefulUpdater::HandleRebootNotificationClick,
        weak_ptr_factory_.GetWeakPtr())),
    vector_icons::kBusinessIcon,
    message_center::SystemNotificationWarningLevel::NORMAL);

  progress_notification_->set_progress(-1);
  progress_notification_->set_pinned(true);

  NotificationDisplayServiceFactory::GetForProfile(profile_)->Display(
      NotificationHandler::Type::TRANSIENT, *progress_notification_,
      /*metadata=*/nullptr);
}

void StatefulUpdater::TrytoUpdateProgressNotification(std::string result) {
  if (!profile_ || !progress_notification_) return;
  VLOG(3) << "stateful update progress: " << result;
  result.erase(std::remove(result.begin(), result.end(), '%'), result.end());
  double progressInDouble;
  if (!base::StringToDouble(result, &progressInDouble)) {
    progress_notification_->set_progress(-1);
  } else {
    int progress = static_cast<int>(progressInDouble);
    if (progress > 0 && progress <= 100) {
      progress_notification_->set_progress(progress);
    } else {
      progress_notification_->set_progress(-1);
    }
  }
  NotificationDisplayServiceFactory::GetForProfile(profile_)->Display(
      NotificationHandler::Type::TRANSIENT, *progress_notification_,
      /*metadata=*/nullptr);
}

void StatefulUpdater::CloseProgressNotification() {
  if (!profile_) return;
  NotificationDisplayServiceFactory::GetForProfile(profile_)->Close(
      NotificationHandler::Type::TRANSIENT,
      kStatefulUpdateProgressNotificationId);
}

void StatefulUpdater::GenerateRebootNotification() {
  std::unique_ptr<Notification> notification =
    ::ash::CreateSystemNotificationPtr(
      message_center::NOTIFICATION_TYPE_SIMPLE,
      kStatefulUpdateRebootNotificationId,
      l10n_util::GetStringUTF16(
        IDS_ASH_FYDEOS_STATEFUL_UPDATE_DONE_NOTIFICATION_TITLE),
      l10n_util::GetStringUTF16(
        IDS_ASH_FYDEOS_STATEFUL_UPDATE_DONE_NOTIFICATION_MESSAGE),
      std::u16string(), GURL(),
      message_center::NotifierId(message_center::NotifierType::SYSTEM_COMPONENT,
                                kStatefulUpdaterNotifierId),
      message_center::RichNotificationData(),
      base::MakeRefCounted<message_center::HandleNotificationClickDelegate>(
        base::BindRepeating(
          &StatefulUpdater::HandleRebootNotificationClick,
          weak_ptr_factory_.GetWeakPtr())),
      vector_icons::kBusinessIcon,
      message_center::SystemNotificationWarningLevel::NORMAL);

  notification->set_pinned(true);
  std::vector<message_center::ButtonInfo> notification_actions;
  notification_actions.emplace_back(l10n_util::GetStringUTF16(
        IDS_ASH_FYDEOS_STATEFUL_UPDATE_DONE_NOTIFICATION_ACTION_REBOOT));

  notification->set_buttons(notification_actions);

  MessageCenter::Get()->AddNotification(std::move(notification));
}

void StatefulUpdater::CloseRebootNotification() {
  message_center::MessageCenter::Get()
    ->RemoveNotification(kStatefulUpdateRebootNotificationId, false);
}

void StatefulUpdater::HandleUpdateNotificationClick(
    absl::optional<int> button_index) {
  if (!button_index) {
    return;
  }
  CloseUpdateNotification();
  if (button_index.value() == 0) {
    DoUpdate();
  }
}

void StatefulUpdater::HandleRebootNotificationClick(
    absl::optional<int> button_index) {
  if (!button_index) {
    return;
  }
  CloseRebootNotification();
  if (button_index.value() == 0) {
    Reboot();
  }
}

void StatefulUpdater::Reboot() {
  chrome::AttemptRelaunch();
}

}  // namespace misc
}  // namespace fydeos
