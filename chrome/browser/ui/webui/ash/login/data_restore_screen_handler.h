// Copyright 2023 Fyde Innovations. All rights reserved
#ifndef CHROME_BROWSER_UI_WEBUI_CHROMEOS_LOGIN_DATA_RESTORE_SCREEN_HANDLER_H_
#define CHROME_BROWSER_UI_WEBUI_CHROMEOS_LOGIN_DATA_RESTORE_SCREEN_HANDLER_H_

#include <string>
#include "chrome/browser/ui/webui/ash/login/base_screen_handler.h"
#include "base/scoped_observation.h"
#include "chrome/browser/ash/usb/cros_usb_detector.h"

namespace ash {

class DataRestoreScreenView {
 public:
  inline constexpr static StaticOobeScreenId kScreenId{"data-restore",
                                                       "DataRestoreScreen"};

  virtual ~DataRestoreScreenView() = default;

  virtual void Show() = 0;
  virtual void Hide() = 0;
  virtual void SetSystemSalt(const std::string& salt) = 0;

  virtual base::WeakPtr<DataRestoreScreenView> AsWeakPtr() = 0;
};

class DataRestoreScreenHandler : public BaseScreenHandler,
                                 public DataRestoreScreenView,
                                 public CrosUsbDeviceObserver {
 public:
  using TView = DataRestoreScreenView;

  DataRestoreScreenHandler();
  DataRestoreScreenHandler(const DataRestoreScreenHandler&) = delete;
  DataRestoreScreenHandler& operator=(const DataRestoreScreenHandler&) = delete;
  ~DataRestoreScreenHandler() override;
  base::WeakPtr<DataRestoreScreenView> AsWeakPtr() override;

 private:
  void DeclareLocalizedValues(
      ::login::LocalizedValuesBuilder* builder) override;
  void Show() override;
  void Hide() override;
  void SetSystemSalt(const std::string& salt) override;

  void OnUsbDevicesChanged() override;

  base::ScopedObservation<CrosUsbDetector, CrosUsbDeviceObserver>
      cros_usb_device_observation_{this};

  base::WeakPtrFactory<DataRestoreScreenHandler> weak_ptr_factory_{this};
};

}  // namespace ash


#endif  // CHROME_BROWSER_UI_WEBUI_CHROMEOS_LOGIN_DATA_RESTORE_SCREEN_HANDLER_H_
