// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fydeos/ash/system/unified/rotate_screen_feature_pod_controller.h"

#include "ash/resources/vector_icons/vector_icons.h"
#include "ash/shell.h"
#include "ash/wm/tablet_mode/tablet_mode_controller.h"
#include "ash/strings/grit/ash_strings.h"
#include "ash/system/unified/feature_pod_button.h"
#include "ash/system/unified/feature_tile.h"
#include "ash/system/unified/unified_system_tray_controller.h"
#include "ui/base/l10n/l10n_util.h"
#include "ash/accelerators/accelerator_controller_impl.h"
#include "base/system/sys_info.h"
#include "fydeos/prefs/fydeos_pref_names.h"
#include "components/prefs/pref_registry_simple.h"
#include "components/prefs/pref_service.h"
#include "fydeos/switches/misc/misc_switches.h"

namespace ash {

namespace {

bool shouldShowRotateScreenByDefault(const std::string& board) {
  return fydeos::switches::IsNonForYouBoard(board);
}

}

RotateScreenFeaturePodController::RotateScreenFeaturePodController() {
  DCHECK(Shell::Get());
  Shell::Get()->tablet_mode_controller()->AddObserver(this);
}

RotateScreenFeaturePodController::~RotateScreenFeaturePodController() {
  if (Shell::Get()->tablet_mode_controller())
    Shell::Get()->tablet_mode_controller()->RemoveObserver(this);
}

std::unique_ptr<FeatureTile> RotateScreenFeaturePodController::CreateTile(
    bool compact) {
  DCHECK(!tile_);
  auto tile = std::make_unique<FeatureTile>(
      base::BindRepeating(&RotateScreenFeaturePodController::OnIconPressed,
                          weak_factory_.GetWeakPtr()));
  tile_ = tile.get();
  tile_->SetVectorIcon(kUnifiedMenuRotateScreenIcon);

  auto label_text = l10n_util::GetStringUTF16(
        IDS_ASH_FYDEOS_UNIFIED_MENU_LABEL_ROTATE_SCREEN);
  tile_->SetLabel(label_text);
  tile_->SetTooltipText(label_text);
  tile_->SetSubLabelVisibility(false);
  UpdateTile();
  return tile;
}

void RotateScreenFeaturePodController::OnIconPressed() {
  RotateScreenFydeOS();
}

void RotateScreenFeaturePodController::OnTabletPhysicalStateChanged() {
  UpdateTile();
}

void RotateScreenFeaturePodController::UpdateTile() {
  const bool is_in_tablet_physical_state =
    Shell::Get()->tablet_mode_controller()->is_in_tablet_physical_state();
  PrefService* prefs = Shell::Get()->local_state();
  bool visible = prefs->GetBoolean(fydeos::prefs::kShowRotateScreenButton);

  if (!tile_) {
    return;
  }
  tile_->SetVisible(is_in_tablet_physical_state && visible);
}

QsFeatureCatalogName RotateScreenFeaturePodController::GetCatalogName() {
  return QsFeatureCatalogName::kRotateScreen;
}

void RotateScreenFeaturePodController::RegisterLocalStatePrefs(
    PrefRegistrySimple* registry) {
  const std::string board = base::SysInfo::GetLsbReleaseBoard();
  registry->RegisterBooleanPref(fydeos::prefs::kShowRotateScreenButton,
                                shouldShowRotateScreenByDefault(board));
}

}  // namespace ash
