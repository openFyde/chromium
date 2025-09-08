// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fydeos/ash/system/unified/switch_tablet_laptop_feature_pod_controller.h"
#include "ash/resources/vector_icons/vector_icons.h"
#include "ash/system/unified/feature_pod_button.h"
#include "ash/system/unified/feature_tile.h"
#include "ash/shell.h"
#include "ash/strings/grit/ash_strings.h"
#include "ui/base/l10n/l10n_util.h"
#include "ash/wm/tablet_mode/tablet_mode_controller.h"
#include "ash/system/unified/unified_system_tray_controller.h"
#include "components/prefs/pref_registry_simple.h"
#include "components/prefs/pref_service.h"
#include "fydeos/prefs/fydeos_pref_names.h"

namespace ash {

SwitchTabletLabtopFeaturePodController::SwitchTabletLabtopFeaturePodController(
    UnifiedSystemTrayController* tray_controller)
    : tray_controller_(tray_controller) {}

SwitchTabletLabtopFeaturePodController::~SwitchTabletLabtopFeaturePodController() = default;

std::unique_ptr<FeatureTile> SwitchTabletLabtopFeaturePodController::CreateTile(
    bool compact) {
  DCHECK(!tile_);
  auto tile = std::make_unique<FeatureTile>(
      base::BindRepeating(
        &SwitchTabletLabtopFeaturePodController::OnIconPressed,
        weak_factory_.GetWeakPtr()));
  tile_ = tile.get();
  tile_->SetSubLabelVisibility(false);
  UpdateTile();
  return tile;
}

void SwitchTabletLabtopFeaturePodController::OnIconPressed() {
  tray_controller_->CloseBubble();
  TabletModeController* controller = Shell::Get()->tablet_mode_controller();
  const bool force_on = controller->IsInFydeForceOnMode();
  const bool force_off = controller->IsInFydeForceOffMode();
  if (!force_on && !force_off && !IsInTabletMode()) {
    // -> default & laptop -> force_on
    controller->SetEnabledByFyde(true);
  } else if (IsInTabletMode()) {
    // -> ((default && tablet) || force_on) -> force_off
    controller->SetEnabledByFyde(false);
  } else if (force_off) {
    // -> force_off -> default
    controller->SetDefaultBehaviorByFyde();
  }
}

QsFeatureCatalogName SwitchTabletLabtopFeaturePodController::GetCatalogName() {
  return QsFeatureCatalogName::kSwitchLaptopTablet;
}

void SwitchTabletLabtopFeaturePodController::RegisterLocalStatePrefs(
    PrefRegistrySimple* registry) {
  registry->RegisterBooleanPref(
      fydeos::prefs::kShowSwitchTabletLaptopButton, false);
}

void SwitchTabletLabtopFeaturePodController::UpdateTile() {
  TabletModeController* controller = Shell::Get()->tablet_mode_controller();
  const bool force_on = controller->IsInFydeForceOnMode();
  const bool force_off = controller->IsInFydeForceOffMode();
  std::u16string label_text;
  const gfx::VectorIcon* icon;
  if (!force_on && !force_off && !IsInTabletMode()) {
    icon = &kUnifiedMenuTabletModeIcon;
    label_text = l10n_util::GetStringUTF16(
        IDS_ASH_FYDEOS_UNIFIED_MENU_LABEL_SWITCH_TO_TABLET_MODE);
  } else if (IsInTabletMode()) {
    icon = &kUnifiedMenuDesktopModeIcon;
    label_text = l10n_util::GetStringUTF16(
        IDS_ASH_FYDEOS_UNIFIED_MENU_LABEL_SWITCH_TO_LAPTOP_MODE);
  } else if (force_off) {
    icon = &kUnifiedMenuDefaultModeIcon;
    label_text = l10n_util::GetStringUTF16(
        IDS_ASH_FYDEOS_UNIFIED_MENU_LABEL_SWITCH_TO_DEFAULT_MODE);
  }
  PrefService* prefs = Shell::Get()->local_state();
  bool visible = prefs->GetBoolean(
      fydeos::prefs::kShowSwitchTabletLaptopButton);
  if (!tile_) {
    return;
  }
  tile_->SetVectorIcon(*icon);
  tile_->SetLabel(label_text);
  tile_->SetTooltipText(label_text);
  tile_->SetVisible(visible);
}

bool SwitchTabletLabtopFeaturePodController::IsInTabletMode() {
  TabletModeController* controller = Shell::Get()->tablet_mode_controller();
  const bool in_dev_mode = controller->IsInDevTabletMode();
  const bool force_on = controller->IsInFydeForceOnMode();
  const bool physical_on = controller->is_in_tablet_physical_state();
  return in_dev_mode || force_on || physical_on;
}

} // namespace ash
