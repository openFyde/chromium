// Copyright 2021 The FydeOS Authors. All rights reserved.

#ifndef ASH_SYSTEM_UNIFIED_SWITCH_TABLET_LAPTOP_FEATURE_POD_CONTROLLER_H_
#define ASH_SYSTEM_UNIFIED_SWITCH_TABLET_LAPTOP_FEATURE_POD_CONTROLLER_H_

#include <memory>
#include "ash/ash_export.h"
#include "ash/system/unified/feature_pod_controller_base.h"
#include "ash/public/cpp/tablet_mode_observer.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"

class PrefRegistrySimple;

namespace ash {

class UnifiedSystemTrayController;

class ASH_EXPORT SwitchTabletLabtopFeaturePodController:
    public FeaturePodControllerBase {
 public:
    explicit SwitchTabletLabtopFeaturePodController(
        UnifiedSystemTrayController* controller);
    ~SwitchTabletLabtopFeaturePodController() override;

    static void RegisterLocalStatePrefs(PrefRegistrySimple* registry);

    // FeaturePodButton* CreateButton() override;
    std::unique_ptr<FeatureTile> CreateTile(bool compact = false) override;
    void OnIconPressed() override;
    QsFeatureCatalogName GetCatalogName() override;

 private:
  void UpdateTile();
  bool IsInTabletMode();

  const raw_ptr<UnifiedSystemTrayController> tray_controller_;

  raw_ptr<FeatureTile, DanglingUntriaged> tile_ = nullptr;

  base::WeakPtrFactory<SwitchTabletLabtopFeaturePodController>
    weak_factory_{this};
};

}  // namespace ash

#endif /* ifndef ASH_SYSTEM_UNIFIED_SWITCH_TABLET_LAPTOP_FEATURE_POD_CONTROLLER_H_ */
