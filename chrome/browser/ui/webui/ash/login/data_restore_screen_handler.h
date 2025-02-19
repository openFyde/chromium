// Copyright 2023 Fyde Innovations. All rights reserved
#ifndef CHROME_BROWSER_UI_WEBUI_CHROMEOS_LOGIN_DATA_RESTORE_SCREEN_HANDLER_H_
#define CHROME_BROWSER_UI_WEBUI_CHROMEOS_LOGIN_DATA_RESTORE_SCREEN_HANDLER_H_

#include "chrome/browser/ui/webui/ash/login/base_screen_handler.h"

namespace ash {

class DataRestoreScreenView :
  public base::SupportsWeakPtr<DataRestoreScreenView> {
 public:
  inline constexpr static StaticOobeScreenId kScreenId{"data-restore",
                                                       "DataRestoreScreen"};

  virtual ~DataRestoreScreenView() = default;

  virtual void Show() = 0;
};

class DataRestoreScreenHandler : public BaseScreenHandler,
                                 public DataRestoreScreenView {
 public:
  using TView = DataRestoreScreenView;

  DataRestoreScreenHandler();
  DataRestoreScreenHandler(const DataRestoreScreenHandler&) = delete;
  DataRestoreScreenHandler& operator=(const DataRestoreScreenHandler&) = delete;
  ~DataRestoreScreenHandler() override;

 private:
  void DeclareLocalizedValues(
      ::login::LocalizedValuesBuilder* builder) override;
  void Show() override;

  base::WeakPtrFactory<DataRestoreScreenHandler> weak_factory_{this};
};

}  // namespace ash


#endif  // CHROME_BROWSER_UI_WEBUI_CHROMEOS_LOGIN_DATA_RESTORE_SCREEN_HANDLER_H_
