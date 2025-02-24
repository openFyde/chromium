// Copyright 2024 The Fyde Innovations Limited. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#ifndef CHROME_BROWSER_UI_WEBUI_SETTINGS_CHROMEOS_FYDE_ASSISTANT_SECTION_H_
#define CHROME_BROWSER_UI_WEBUI_SETTINGS_CHROMEOS_FYDE_ASSISTANT_SECTION_H_

#include <string>
#include "base/values.h"
#include "chrome/browser/ui/webui/ash/settings/pages/os_settings_sections.h"
#include "chrome/browser/ui/webui/ash/settings/search/search_tag_registry.h"

class PrefService;
class Profile;

namespace ash::settings {

class FydeAssistantSection : public OsSettingsSection {
 public:
  FydeAssistantSection(Profile* profile, SearchTagRegistry* search_tag_registry,
      PrefService* pref_service);
  ~FydeAssistantSection() override;

private:
  // OsSettingsSection:
  void AddLoadTimeData(content::WebUIDataSource* html_source) override;
  void AddHandlers(content::WebUI* web_ui) override;
  int GetSectionNameMessageId() const override;
  chromeos::settings::mojom::Section GetSection() const override;
  ash::settings::mojom::SearchResultIcon GetSectionIcon() const override;
  const char* GetSectionPath() const override;
  bool LogMetric(chromeos::settings::mojom::Setting setting,
                 base::Value& value) const override;
  void RegisterHierarchy(HierarchyGenerator* generator) const override;

  PrefService* pref_service_;
};

} // namespace ash::settings

#endif // CHROME_BROWSER_UI_WEBUI_SETTINGS_CHROMEOS_FYDE_ASSISTANT_SECTION_H_
