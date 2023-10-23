// Copyright (c) 2021 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fydeos/ui/webui/settings/ash/fydeos_section.h"
#include "base/no_destructor.h"
#include "chrome/grit/generated_resources.h"
#include "chrome/grit/chromium_strings.h"
#include "ui/base/l10n/l10n_util.h"
#include "content/public/browser/web_ui.h"
#include "chrome/browser/ui/webui/webui_util.h"
#include "ui/base/webui/web_ui_util.h"
#include "content/public/browser/web_ui_data_source.h"
#include "chrome/browser/profiles/profile.h"
#include "components/prefs/pref_service.h"
#include "base/system/sys_info.h"
#include "chrome/browser/browser_process.h"
#include "fydeos/switches/misc/misc_switches.h"
#include "fydeos/switches/urls/urls_constants.h"

#include "base/strings/utf_string_conversions.h"

#include "fydeos/ui/webui/settings/ash/fydeos_handler.h"
#include "fydeos/prefs/fydeos_pref_names.h"

namespace ash::settings {

namespace mojom {
using ::chromeos::settings::mojom::kFydeOsSectionPath;
using ::chromeos::settings::mojom::kFydeOsSubpagePath;
using ::chromeos::settings::mojom::Section;
using ::chromeos::settings::mojom::Subpage;
using ::chromeos::settings::mojom::Setting;
}

namespace {
  const std::vector<SearchConcept>& GetFydeOsSearchConcepts() {
    static const base::NoDestructor<std::vector<SearchConcept>> tags({
      {IDS_OS_SETTINGS_FYDEOS_SETTINGS,
       mojom::kFydeOsSubpagePath,
       mojom::SearchResultIcon::kChrome,
       mojom::SearchResultDefaultRank::kMedium,
       mojom::SearchResultType::kSubpage,
       {.subpage = mojom::Subpage::kFydeOsMain}},
    });
    return *tags;
  }
}

FydeOsSection::FydeOsSection(Profile* profile,
                           SearchTagRegistry* search_tag_registry,
                           PrefService* pref_service)
  : OsSettingsSection(profile, search_tag_registry),
    pref_service_(pref_service) {
  SearchTagRegistry::ScopedTagUpdater updater = registry()->StartUpdate();
  updater.AddSearchTags(GetFydeOsSearchConcepts());
}

FydeOsSection::~FydeOsSection() = default;

void FydeOsSection::AddLoadTimeData(content::WebUIDataSource* html_source) {
  static constexpr webui::LocalizedString kLocalizedStrings[] = {
    {"fydeosSettingsAccountTitle", IDS_OS_SETTINGS_FYDEOS_ACCOUNT_TITLE},
    {"fydeosSettingsWifiDriverTitle", IDS_OS_SETTINGS_FYDEOS_WIFI_DRIVER_TITLE},
    {"fydeosSettingsWifiDriverLabel",
      IDS_FYDEOS_ADVANCED_SETTING_WIFI_DRIVER_TITLE},
    {"fydeosSettingsWifiDriverDesc",
      IDS_FYDEOS_ADVANCED_SETTING_WIFI_DRIVER_HINT},
    {"fydeosSettingsWifiDriverNote",
      IDS_FYDEOS_ADVANCED_SETTING_WIFI_DRIVER_HINT2},
    {"fydeosSettingsTouchpadModeTitle",
      IDS_OS_SETTINGS_FYDEOS_TOUCHPAD_MODE_TITLE},
    {"fydeosSettingsTouchpadModeLabel",
      IDS_FYDEOS_ADVANCED_SETTING_TOUCHPAD_DRIVER_TITLE},
    {"fydeosSettingsTouchpadModeDesc",
      IDS_FYDEOS_ADVANCED_SETTING_TOUCHPAD_DRIVER_HINT},
    {"fydeosSettingsTouchpadModeNote",
      IDS_FYDEOS_ADVANCED_SETTING_TOUCHPAD_DRIVER_HINT1},
    {"fydeosSettingsRemoteAssistanceTitle",
      IDS_OS_SETTINGS_FYDEOS_REMOTE_ASSISTANCE_TITLE},
    {"fydeosSettingsRemoteHelperServiceTitle",
      IDS_OS_SETTINGS_FYDEOS_REMOTE_HELPER_SERVICE_TITLE},
    {"fydeosSettingsRemoteHelperDesc",
      IDS_OS_SETTINGS_FYDEOS_REMOTE_HELPER_SERVICE_DESC},
    {"fydeosSettingsRemoteHelperEnabledMessage",
      IDS_OS_SETTINGS_FYDEOS_REMOTE_HELPER_SERVICE_ENABLED_MESSAGE},
    {"fydeosSettingsRemoteHelperRequireRestartMessage",
      IDS_OS_SETTINGS_FYDEOS_REMOTE_HELPER_SERVICE_REQUIRE_RESTART_MESSAGE},
    {"fydeosSettingsRemoteHelperStartingMessage",
      IDS_OS_SETTINGS_FYDEOS_REMOTE_HELPER_SERVICE_STARGING_MESSAGE},
    {"fydeosSettingsMoreInfoTitle", IDS_OS_SETTINGS_FYDEOS_MORE_INFO_TITLE},

    {"fydeosSettingsOtherTweaksTitle",
      IDS_OS_SETTINGS_FYDEOS_OTHER_TWEAKS_TITLE},
    {"rebootButtonInTrayLabel",
      IDS_OS_SETTINGS_FYDEOS_REBOOT_BUTTON_IN_TRAY_LABEL},
    {"displayFydeOsRebootButtonInTray",
      IDS_OS_SETTINGS_FYDEOS_DISPLAY_REBOOT_BUTTON_IN_TRAY},
    {"rotateScreenButtonInTrayLabel",
      IDS_OS_SETTINGS_FYDEOS_ROTATE_SCREEN_BUTTON_IN_TRAY_LABEL},
    {"notTabletPhysicalStateDisableFydeOsRotateScreen",
      IDS_OS_SETTINGS_FYDEOS_NOT_TABLET_STATE_DISABLE_ROTATE_SCREEN},
    {"displayFydeOsRotateScreenButton",
      IDS_OS_SETTINGS_FYDEOS_DISPLAY_ROTATE_SCREEN_BUTTON},

    {"switchTabletLaptopModeButtonInTrayLabel",
      IDS_OS_SETTINGS_FYDEOS_SWITCH_TABLET_LAPTOP_MODE_BUTTON_IN_TRAY_LABEL},
    {"displaySwitchTabletLaptopModeButton",
      IDS_OS_SETTINGS_FYDEOS_DISPLAY_SWITCH_TABLET_LAPTOP_MODE_BUTTON},

    {"enableLibwidevineLabel", IDS_OS_SETTINGS_FYDEOS_ENABLE_LIBWIDEVINE_LABEL},
    {"failedEnableWidevineTitle",
      IDS_OS_SETTINGS_FYDEOS_FAILED_ENABLE_WIDEVINE_TITLE},
    {"failedEnableWidevineMessage",
      IDS_OS_SETTINGS_FYDEOS_FAILED_ENABLE_WIDEVINE_MESSAGE},

    {"fydeosExperimentalFeatures",
      IDS_OS_SETTINGS_FYDEOS_EXPERIMENTAL_FEATURES_TITLE},
    {"fydeosBypassTpmChecksTitle",
      IDS_OS_SETTINGS_FYDEOS_BYPASS_TPM_CHECKS_TITLE},
    {"fydeosBypassTpmChecksDesc",
      IDS_OS_SETTINGS_FYDEOS_BYPASS_TPM_CHECKS_DESC},

    {"autoSigninForFydeLocalAccountTitle",
      IDS_OS_SETTINGS_FYDEOS_AUTO_SIGNIN_FOR_LOCAL_ACCOUNT_TITLE},
    {"enableAutoSigninForFydeLocalAccountHelpMessage",
      IDS_OS_SETTINGS_FYDEOS_ENABLE_AUTO_SIGNIN_FOR_LOCAL_ACCOUNT_MESSAGE},
    {"autoSigninForFydeLocalAccountOtherUserAlreadyEnabled",
      IDS_OS_SETTINGS_FYDEOS_AUTO_SIGNIN_FOR_LOCAL_ACCOUNT_ALREADY_ENABLED_BY_OTHER},
    {"unableToSetAutoSigninForFydeLocalAccount",
      IDS_OS_SETTINGS_FYDEOS_UNABLE_TO_SET_AUTO_SIGNIN_FOR_LOCAL_ACCOUNT},
    {"unableToSetAutoSigninForFydeNonLocalAccount",
      IDS_OS_SETTINGS_FYDEOS_UNABLE_TO_SET_AUTO_SIGNIN_FOR_NON_LOCAL_ACCOUNT},

    {"fydeosSettingsBackupButtonLabel",
      IDS_OS_SETTINGS_FYDEOS_BACKUP_BUTTON_LABEL},
    {"fydeosSettingsBackupIntroTitle",
      IDS_OS_SETTINGS_FYDEOS_BACKUP_INTRO_TITLE},
    {"fydeosSettingsBackupPasswordPromptTitle",
      IDS_OS_SETTINGS_FYDEOS_BACKUP_PASSWORD_PROMPT_TITLE},
    {"fydeosSettingsBackupPasswordPromptText",
      IDS_OS_SETTINGS_FYDEOS_BACKUP_PASSWORD_PROMPT_TEXT},
  };

  html_source->AddLocalizedStrings(kLocalizedStrings);
  html_source->AddString("fydeosSettingsPageTitle",
      l10n_util::GetStringFUTF16(IDS_OS_SETTINGS_FYDEOS_SETTINGS,
        l10n_util::GetStringUTF16(IDS_PRODUCT_OS_NAME)));

  html_source->AddString(
      "fydeosSettingsBackupLabel",
      l10n_util::GetStringFUTF16(IDS_OS_SETTINGS_FYDEOS_BACKUP_LABEL,
          base::ASCIIToUTF16(
            fydeos::constants::kFydeOSBackupRestoreLearnMoreURL)));
  html_source->AddString(
      "fydeosSettingsBackupIntroText",
      l10n_util::GetStringFUTF16(
          IDS_OS_SETTINGS_FYDEOS_BACKUP_INTRO_TEXT,
          base::ASCIIToUTF16(
            fydeos::constants::kFydeOSBackupRestoreLearnMoreURL)));

  html_source->AddString(
      "toggleWidevineHelpMessage",
      l10n_util::GetStringFUTF16(
          IDS_OS_SETTINGS_FYDEOS_TOGGLE_LIBWIDEVINE_HELP_MESSAGE,
            base::ASCIIToUTF16(
              fydeos::constants::kFydeOSEnableWidevineLearnMoreURL)));

  const std::string board = base::SysInfo::GetLsbReleaseBoard();
  html_source->AddBoolean("showToggleRebootButtonInTray", true);
  html_source->AddBoolean("showToggleRotateScreenButton",
      fydeos::switches::IsNonForYouBoard(board));
  html_source->AddBoolean("showToggleSwitchTabletLaptopButton", true);

  html_source->AddString("fydeOSRdpUrl",
      fydeos::constants::kFydeOSRemoteDesktopURL);
  html_source->AddString("fydeosAccountBaseUrl",
      fydeos::constants::kFydeOSAccountBaseUrl);

  html_source->AddBoolean("isTpmFallbackNecessary",
      g_browser_process->local_state()->GetBoolean(
        fydeos::prefs::kForceTpmFallbackNecessary));
  html_source->AddString("fydeExperimentTpmfallbackUrl",
      fydeos::constants::kFydeExperimentTpmFallbackUrl);
}

int FydeOsSection::GetSectionNameMessageId() const {
  return IDS_OS_SETTINGS_FYDEOS_SETTINGS;
}

mojom::Section FydeOsSection::GetSection() const {
  return mojom::Section::kFydeOs;
}

mojom::SearchResultIcon FydeOsSection::GetSectionIcon() const {
  NOTIMPLEMENTED();
  return mojom::SearchResultIcon::kChrome;
}

std::string FydeOsSection::GetSectionPath() const {
  return mojom::kFydeOsSectionPath;
}

void FydeOsSection::AddHandlers(content::WebUI* web_ui) {
  web_ui->AddMessageHandler(
      std::make_unique<FydeOsHandler>(profile(), pref_service_));
}

bool FydeOsSection::LogMetric(
    mojom::Setting setting, base::Value& value) const {
  // Unimplemented.
  return false;
}

void FydeOsSection::RegisterHierarchy(HierarchyGenerator* generator) const {
  // fydeos top level
  generator->RegisterTopLevelSubpage(
      IDS_OS_SETTINGS_FYDEOS_SETTINGS, mojom::Subpage::kFydeOsMain,
      mojom::SearchResultIcon::kChrome, mojom::SearchResultDefaultRank::kMedium,
      mojom::kFydeOsSubpagePath);
}

}  // namespace ash::settings
