// Copyright 2023 Fyde Innovations. All rights reserved
#ifndef CHROME_BROWSER_ASH_LOGIN_SCREENS_DATA_RESTORE_SCREEN_H_
#define CHROME_BROWSER_ASH_LOGIN_SCREENS_DATA_RESTORE_SCREEN_H_

#include "base/functional/callback.h"
#include "base/memory/weak_ptr.h"
#include "chrome/browser/ash/login/screens/base_screen.h"
#include "chrome/browser/ui/webui/ash/login/data_restore_screen_handler.h"

namespace ash {

class DataRestoreScreen : public BaseScreen {
 public:
    DataRestoreScreen(base::WeakPtr<DataRestoreScreenView> view,
                      const base::RepeatingClosure& exit_callback);
    ~DataRestoreScreen() override;
    DataRestoreScreen(const DataRestoreScreen&) = delete;
    DataRestoreScreen& operator=(const DataRestoreScreen&) = delete;

 private:
    // BaseScreen:
    void ShowImpl() override;
    void HideImpl() override;
    void OnUserAction(const base::Value::List& args) override;

    base::WeakPtr<DataRestoreScreenView> view_;
    base::RepeatingClosure exit_callback_;
};

}  // namespace ash


#endif  // CHROME_BROWSER_ASH_LOGIN_SCREENS_DATA_RESTORE_SCREEN_H_
