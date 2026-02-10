// Copyright 2024 The Fyde Innovations Limited. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#include "fydeos/misc/fydeos_crostini_notifier.h"
#include "base/task/single_thread_task_runner.h"
#include "chrome/browser/ash/crostini/crostini_util.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "chrome/browser/ash/crostini/crostini_features.h"
#include "components/prefs/pref_service.h"
#include "fydeos/prefs/fydeos_pref_names.h"
#include "ui/message_center/message_center.h"
#include "ui/message_center/public/cpp/notification.h"
#include "ash/public/cpp/notification_utils.h"
#include "components/vector_icons/vector_icons.h"
#include "ui/base/l10n/l10n_util.h"
#include "ash/strings/grit/ash_strings.h"
#include "chrome/browser/ash/crostini/crostini_installer.h"
#include "chrome/browser/ash/crostini/crostini_installer_factory.h"
#include "base/logging.h"

using message_center::MessageCenter;
using message_center::Notification;

namespace fydeos::misc {

namespace {
  const char kMiscCrostiniInstallNotificationId[] =
    "fydeos.misc.crostini.install.notification";
  const char kMiscCrostiniInstallNotifierId[] =
    "fydeos.misc.crostini.install.notifier";

  const int kCrostiniInstallerNotificationDelayInSeconds = 10;
}

MiscCrostiniNotifier::MiscCrostiniNotifier() = default;
MiscCrostiniNotifier::~MiscCrostiniNotifier() = default;

void MiscCrostiniNotifier::Start() {
  if (started_) return;
  started_ = true;
  base::SingleThreadTaskRunner::GetCurrentDefault()->PostDelayedTask(
      FROM_HERE,
      base::BindOnce(&MiscCrostiniNotifier::MayShowCrostiniInstallerNotification,
                     weak_ptr_factory_.GetWeakPtr()),
      base::Seconds(kCrostiniInstallerNotificationDelayInSeconds));
}

void MiscCrostiniNotifier::CloseNotification() {
  message_center::MessageCenter::Get()
    ->RemoveNotification(kMiscCrostiniInstallNotificationId, false);
}

void MiscCrostiniNotifier::MayShowCrostiniInstallerNotification() {
  if (g_browser_process && g_browser_process->profile_manager()) {
    profile_ = g_browser_process->profile_manager()->GetActiveUserProfile();
  }
  if (!profile_) {
    return;
  }
  // if crostini not allowd, return
  if (!crostini::CrostiniFeatures::Get()->IsAllowedNow(profile_)) {
    VLOG(2) << "Crostini not allowed for current profile";
    CloseNotification();
    return;
  }
  // if crostini already enabled, return
  if (crostini::CrostiniFeatures::Get()->IsEnabled(profile_)) {
    VLOG(2) << "Crostini already enabled for current profile";
    CloseNotification();
    return;
  }
  // if already interacted, return
  bool notified = profile_->GetPrefs()->GetBoolean(fydeos::prefs::kCrostiniInstallerNotificationUserInteracted);
  if (notified) {
    VLOG(2) << "Crostini installer notification already shown and clicked(or closed) by current user";
    return;
  }
  std::unique_ptr<Notification> notification =
    ::ash::CreateSystemNotificationPtr(
      message_center::NOTIFICATION_TYPE_SIMPLE,
      kMiscCrostiniInstallNotificationId,
      l10n_util::GetStringUTF16(
        IDS_ASH_FYDEOS_CROSTINI_INSTALL_NOTIFICATION_TITLE),
      l10n_util::GetStringUTF16(
        IDS_ASH_FYDEOS_CROSTINI_INSTALL_NOTIFICATION_MESSAGE),
      std::u16string(), GURL(),
      message_center::NotifierId(
        message_center::NotifierType::SYSTEM_COMPONENT,
        kMiscCrostiniInstallNotifierId,
        ::ash::NotificationCatalogName::kFydeOSCrostiniInstall),
      message_center::RichNotificationData(),
      base::MakeRefCounted<message_center::ThunkNotificationDelegate>(
          weak_ptr_factory_.GetWeakPtr()),
      vector_icons::kProductIcon,
      message_center::SystemNotificationWarningLevel::NORMAL);

  MessageCenter::Get()->AddNotification(std::move(notification));
}

void MiscCrostiniNotifier::Click(
    const std::optional<int>& button_index,
    const std::optional<std::u16string>& reply) {
  if (!profile_) return;
  CloseNotification();
  profile_->GetPrefs()->SetBoolean(fydeos::prefs::kCrostiniInstallerNotificationUserInteracted, true);
  auto installer = crostini::CrostiniInstallerFactory::GetForProfile(profile_);
  if (installer) {
    installer->ShowDialog(crostini::CrostiniUISurface::kNotification);
  }
}

void MiscCrostiniNotifier::Close(bool by_user) {
  if (!profile_) return;
  if (by_user) {
    profile_->GetPrefs()->SetBoolean(fydeos::prefs::kCrostiniInstallerNotificationUserInteracted, true);
  }
}

} // namespace fydeos::misc
