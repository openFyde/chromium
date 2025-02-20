// Copyright 2020 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fydeos/license/fydeos_license_enforcement.h"

#include "base/timer/timer.h"
#include "chrome/browser/ui/browser_commands.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "chrome/browser/apps/app_service/app_launch_params.h"
#include "chrome/browser/ui/extensions/application_launch.h"
#include "chrome/common/extensions/manifest_handlers/app_launch_info.h"
#include "chrome/browser/lifetime/application_lifetime.h"
#include "ash/public/cpp/notification_utils.h"
#include "chrome/browser/notifications/notification_handler.h"
#include "chrome/browser/notifications/notification_display_service.h"
#include "base/strings/utf_string_conversions.h"
#include "ui/message_center/message_center.h"
#include "content/public/browser/web_contents.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/browser_window.h"
#include "ui/base/l10n/l10n_util.h"
#include "ash/strings/grit/ash_strings.h"
#include "chrome/browser/notifications/notification_display_service_factory.h"
#include "fydeos/switches/license/license_switches.h"
#include "net/base/url_util.h"
#include "ash/constants/notifier_catalogs.h"

namespace fydeos::license {
namespace {
  const char kFydeOSLicensePopupPath[] = "/web/checkoutCounter.html";
  const char kFydeOSLicenseForceQuitNotificationId[] =
    "fydeos.license.enforcement.forcequit";
  const char kFydeOSLicenseEnforceonmentNotifierId[] =
    "fydeos.license-enforcement";
  const int kFydeOSForceQuitDelayInMinutes = 15;
  const int kFydeOSRefreshForceQuitNotificationIntervalInSeconds = 10;
  const int kFydeOSEnforceIntervalInMinutes = 10;

  const GURL GetLicensePageUrl(
      const std::string& licenseId, const std::string& serialNumber) {
    GURL url(fydeos::switches::GetFydeOSLicenseWebUrl() + kFydeOSLicensePopupPath);
    url = net::AppendQueryParameter(url, "licenseId", licenseId);
    url = net::AppendQueryParameter(url, "serialNumber", serialNumber);
    const std::string locale = g_browser_process->GetApplicationLocale();
    url = net::AppendQueryParameter(url, "hl", locale);
    return url;
  }
}  // namespace

LicenseEnforcement::LicenseEnforcement():
    enforce_timer_(std::make_unique<base::RepeatingTimer>()),
    force_quit_timer_(std::make_unique<base::OneShotTimer>()),
    notification_timer_(std::make_unique<base::RepeatingTimer>()),
    webContents_(nullptr),
    profile_(nullptr) {}

LicenseEnforcement::~LicenseEnforcement() = default;

void LicenseEnforcement::StartEnforcement(const std::string& licenseID,
                                          const std::string& serialNumber,
                                          EnforcementMode mode) {
  id_ = licenseID;
  serial_number_ = serialNumber;
  mode_ = mode;
  profile_ = g_browser_process->profile_manager()->GetActiveUserProfile();

  if (enforce_timer_->IsRunning()) {
    if (fydeos::switches::IsLicenseTestMode()) {
      Enforce();
    }
    return;
  }

  enforce_timer_->Start(
      FROM_HERE,
      base::Minutes(kFydeOSEnforceIntervalInMinutes),
      base::BindRepeating(&LicenseEnforcement::Enforce,
                          base::Unretained(this)));

  Enforce();
}

void LicenseEnforcement::Enforce() {
  if (mode_ > EnforcementModeLevel0) {
    PopupLicenseWindow();
  }

  if (mode_ > EnforcementModeLevel1) {
    ForceQuitCurrentUser();
  }
}

void LicenseEnforcement::StopEnforcement() {
  if (force_quit_timer_->IsRunning()) {
    force_quit_timer_->Stop();
  }
  if (notification_timer_->IsRunning()) {
    notification_timer_->Stop();
  }
  if (enforce_timer_->IsRunning()) {
    enforce_timer_->Stop();
  }
  CloseLicenseWindow();
  RemoveForceQuitNotification();
}

void LicenseEnforcement::PopupLicenseWindow() {
  if (!profile_) return;

  VLOG(2) << "FydeOS License Enforcement, PopupLicenseWindow";
  Browser* browser = nullptr;
  if (webContents_) {
    browser = chrome::FindBrowserWithTab(webContents_);
  }

  if (!browser) {
    const GURL url = GetLicensePageUrl(id_, serial_number_);
    webContents_ = OpenAppShortcutWindow(profile_, url);
  } else {
    browser->window()->Activate();
  }
}

void LicenseEnforcement::CloseLicenseWindow() {
  if (!profile_) return;
  Browser* browser = nullptr;
  if (webContents_) {
    browser = chrome::FindBrowserWithTab(webContents_);
  }
  if (browser) {
    browser->window()->Close();
  }
}

void LicenseEnforcement::ForceQuitCurrentUser() {
  if (!profile_) return;

  if (force_quit_timer_->IsRunning()) {
    PopupForceQuitNotification();
    return;
  }

  VLOG(2) << "FydeOS LicenseEnforcement, logout after "
          << kFydeOSForceQuitDelayInMinutes << " minutes";
  ForceQuitNotification();
  force_quit_timer_->Start(
      FROM_HERE,
      base::Minutes(kFydeOSForceQuitDelayInMinutes),
      base::BindOnce(&LicenseEnforcement::KickOut, base::Unretained(this)));
}

void LicenseEnforcement::KickOut() {
  StopEnforcement();
  chrome::AttemptUserExit();
}

void LicenseEnforcement::OnForceQuitNotificationClicked() {
  PopupLicenseWindow();
}

void LicenseEnforcement::ForceQuitNotification() {
  notification_ = ash::CreateSystemNotificationPtr(
      message_center::NOTIFICATION_TYPE_SIMPLE,
      kFydeOSLicenseForceQuitNotificationId,
      l10n_util::GetStringUTF16(
        IDS_ASH_FYDEOS_ENFORCEMENT_FORCE_QUIT_NOTIFICATION_TITLE),
      std::u16string(), std::u16string(), GURL(),
      message_center::NotifierId(
          message_center::NotifierType::SYSTEM_COMPONENT,
          kFydeOSLicenseEnforceonmentNotifierId,
          ::ash::NotificationCatalogName::kFydeOSLicenseEnforcement),
      message_center::RichNotificationData(),
      new message_center::HandleNotificationClickDelegate(
          base::BindRepeating(
            &LicenseEnforcement::OnForceQuitNotificationClicked,
            base::Unretained(this))),
      gfx::kNoneIcon,
      message_center::SystemNotificationWarningLevel::CRITICAL_WARNING);
  notification_->SetSystemPriority();
  notification_->set_pinned(true);
  notification_->set_fullscreen_visibility(
      message_center::FullscreenVisibility::OVER_USER);

  notification_timer_->Start(
      FROM_HERE,
      base::Seconds(kFydeOSRefreshForceQuitNotificationIntervalInSeconds),
      base::BindRepeating(
        &LicenseEnforcement::UpdateForceQuitNotification,
        base::Unretained(this)));

  UpdateForceQuitNotification();
}

std::u16string LicenseEnforcement::ForceQuitNotificationMessage() {
  base::TimeDelta left =
    force_quit_timer_->desired_run_time() - base::TimeTicks::Now();
  int seconds = left.InSeconds();
  return l10n_util::GetStringFUTF16(
      IDS_ASH_FYDEOS_ENFORCEMENT_FORCE_QUIT_NOTIFICATION_MESSAGE,
      base::NumberToString16(
        seconds > 0 ? seconds : kFydeOSForceQuitDelayInMinutes * 60));
}

void LicenseEnforcement::UpdateForceQuitNotification() {
  if (notification_ == nullptr) return;
  notification_->set_message(ForceQuitNotificationMessage());
  notification_->set_renotify(false);

  NotificationDisplayServiceFactory::GetForProfile(profile_)->Display(
      NotificationHandler::Type::TRANSIENT, *notification_,
      /*metadata=*/nullptr);
}

void LicenseEnforcement::PopupForceQuitNotification() {
  if (notification_ == nullptr) {
    return;
  }
  notification_->set_renotify(true);

  NotificationDisplayServiceFactory::GetForProfile(profile_)->Display(
      NotificationHandler::Type::TRANSIENT, *notification_,
      /*metadata=*/nullptr);
}

void LicenseEnforcement::RemoveForceQuitNotification() {
  if (!profile_ || notification_ == nullptr) return;
  NotificationDisplayServiceFactory::GetForProfile(profile_)->Close(
      NotificationHandler::Type::TRANSIENT,
      kFydeOSLicenseForceQuitNotificationId);
  notification_.reset();
}

}  // namespace fydeos::license
