// Copyright (c) 2021 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_SETTINGS_CHROMEOS_FYDEOS_SECTION_H_
#define CHROME_BROWSER_UI_WEBUI_SETTINGS_CHROMEOS_FYDEOS_SECTION_H_

#include <string>
#include "base/values.h"
#include "chrome/browser/ui/webui/settings/ash/os_settings_section.h"
#include "chrome/browser/ui/webui/settings/ash/search/search_tag_registry.h"

class PrefService;
class Profile;

namespace ash::settings {

class FydeOsSection : public OsSettingsSection {
 public:
  FydeOsSection(Profile* profile, SearchTagRegistry* search_tag_registry,
      PrefService* pref_service);
  ~FydeOsSection() override;

 private:
  // OsSettingsSection:
  void AddLoadTimeData(content::WebUIDataSource* html_source) override;
  void AddHandlers(content::WebUI* web_ui) override;
  int GetSectionNameMessageId() const override;
  chromeos::settings::mojom::Section GetSection() const override;
  ash::settings::mojom::SearchResultIcon GetSectionIcon() const override;
  std::string GetSectionPath() const override;
  bool LogMetric(chromeos::settings::mojom::Setting setting,
                 base::Value& value) const override;
  void RegisterHierarchy(HierarchyGenerator* generator) const override;

  PrefService* pref_service_;
};

}  // namespace ash::settings

#endif
