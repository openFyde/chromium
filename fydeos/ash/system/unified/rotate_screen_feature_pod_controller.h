// Copyright 2021 The FydeOS Authors. All rights reserved.

#ifndef ASH_SYSTEM_UNIFIED_ROTATE_SCREEN_FEATURE_POD_CONTROLLER_H_
#define ASH_SYSTEM_UNIFIED_ROTATE_SCREEN_FEATURE_POD_CONTROLLER_H_

#include <memory>
#include "ash/ash_export.h"
#include "ash/constants/quick_settings_catalogs.h"
#include "ash/public/cpp/tablet_mode_observer.h"
#include "ash/system/unified/feature_pod_controller_base.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"

class PrefRegistrySimple;

namespace ash {

class ASH_EXPORT RotateScreenFeaturePodController :
    public FeaturePodControllerBase,
    public TabletModeObserver {
 public:
  RotateScreenFeaturePodController();
  ~RotateScreenFeaturePodController() override;

  static void RegisterLocalStatePrefs(PrefRegistrySimple* registry);

  // FeaturePodControllerBase:
  FeaturePodButton* CreateButton() override;
  std::unique_ptr<FeatureTile> CreateTile(bool compact = false) override;
  void OnIconPressed() override;
  QsFeatureCatalogName GetCatalogName() override;

  // TabletModeObserver:
  void OnTabletPhysicalStateChanged() override;

 private:
  void UpdateButton();

  raw_ptr<FeaturePodButton, ExperimentalAsh> button_ = nullptr;
  raw_ptr<FeatureTile, ExperimentalAsh> tile_ = nullptr;

  base::WeakPtrFactory<RotateScreenFeaturePodController> weak_factory_{this};
};

}  // namespace ash

#endif  // ASH_SYSTEM_UNIFIED_ROTATE_SCREEN_FEATURE_POD_CONTROLLER_H_
