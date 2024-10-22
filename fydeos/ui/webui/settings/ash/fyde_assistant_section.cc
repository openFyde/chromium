// Copyright 2024 The Fyde Innovations Limited. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#include "fydeos/ui/webui/settings/ash/fyde_assistant_section.h"

#include "ash/constants/ash_features.h"
#include "chrome/browser/profiles/profile.h"
#include "components/prefs/pref_service.h"
#include "chrome/grit/generated_resources.h"
#include "content/public/browser/web_ui.h"
#include "content/public/browser/web_ui_data_source.h"

namespace ash::settings {

namespace mojom {
using ::chromeos::settings::mojom::kFydeAssistantSectionPath;
using ::chromeos::settings::mojom::Section;
using ::chromeos::settings::mojom::Subpage;
using ::chromeos::settings::mojom::Setting;
} // namespace mojom

namespace {
  const std::vector<SearchConcept>& GetFydeAssistantSearchConcepts() {
    static const base::NoDestructor<std::vector<SearchConcept>> tags({
      {IDS_OS_SETTINGS_FYDE_ASSISTANT,
       mojom::kFydeAssistantSectionPath,
       mojom::SearchResultIcon::kAssistant,
       mojom::SearchResultDefaultRank::kMedium,
       mojom::SearchResultType::kSetting,
       {.setting = mojom::Setting::kFydeAssistantSettings}},
    });
    return *tags;
  }
}

FydeAssistantSection::FydeAssistantSection(Profile* profile,
                                           SearchTagRegistry* search_tag_registry,
                                           PrefService* pref_service)
  : OsSettingsSection(profile, search_tag_registry),
    pref_service_(pref_service) {
  SearchTagRegistry::ScopedTagUpdater updater = registry()->StartUpdate();
  updater.AddSearchTags(GetFydeAssistantSearchConcepts());
}

FydeAssistantSection::~FydeAssistantSection() = default;

void FydeAssistantSection::AddLoadTimeData(content::WebUIDataSource* html_source) {
  static constexpr webui::LocalizedString kLocalizedStrings[] = {
    {"fydeAssistantPageTitle", IDS_OS_SETTINGS_FYDE_ASSISTANT},
    {"fydeAssistantMenuItemDescription", IDS_OS_SETTINGS_FYDE_ASSISTANT_MENU_ITEM_DESCRIPTION},
    {"fydeAssistantToggleLabel", IDS_OS_SETTINGS_FYDE_ASSISTANT_TOGGLE_LABEL},
    {"fydeAssistantToggleSublabel", IDS_OS_SETTINGS_FYDE_ASSISTANT_TOGGLE_SUBLABEL},
    {"fydeAssistantBubbleShortcutToggleLabel", IDS_OS_SETTINGS_FYDE_ASSISTANT_BUBBLE_SHORTCUT_TOGGLE_LABEL},
    {"fydeAssistantBubbleShortcutToggleSublabel", IDS_OS_SETTINGS_FYDE_ASSISTANT_BUBBLE_SHORTCUT_TOGGLE_SUBLABEL},
    {"fydeAssistantSettingsInAppLabel", IDS_OS_SETTINGS_FYDE_ASSISTANT_SETTINGS_IN_APP_LABEL},
    {"fydeAssistantSettingsInAppSublabel", IDS_OS_SETTINGS_FYDE_ASSISTANT_SETTINGS_IN_APP_SUBLABEL},
  };
  html_source->AddLocalizedStrings(kLocalizedStrings);
  html_source->AddBoolean("fydeAssistantFeatureEnabled",ash::features::IsFydeAssistantEnabled());
}

void FydeAssistantSection::AddHandlers(content::WebUI* web_ui) {

}

int FydeAssistantSection::GetSectionNameMessageId() const {
  return IDS_OS_SETTINGS_FYDE_ASSISTANT;
}
chromeos::settings::mojom::Section FydeAssistantSection::GetSection() const {
  return mojom::Section::kFydeAssistant;
}

ash::settings::mojom::SearchResultIcon FydeAssistantSection::GetSectionIcon() const {
  return mojom::SearchResultIcon::kAssistant;
}

const char* FydeAssistantSection::GetSectionPath() const {
  return mojom::kFydeAssistantSectionPath;
}

bool FydeAssistantSection::LogMetric(chromeos::settings::mojom::Setting setting,
               base::Value& value) const {
  return false;
}

void FydeAssistantSection::RegisterHierarchy(HierarchyGenerator* generator) const {
  generator->RegisterTopLevelSetting(mojom::Setting::kFydeAssistantSettings);
}

} // namespace ash::settings
