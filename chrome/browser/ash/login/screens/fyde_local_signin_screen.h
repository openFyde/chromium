// Copyright 2022 Fyde Innovations. All rights reserved

#ifndef CHROME_BROWSER_ASH_LOGIN_SCREENS_FYDE_LOCAL_SIGNIN_SCREEN_H_
#define CHROME_BROWSER_ASH_LOGIN_SCREENS_FYDE_LOCAL_SIGNIN_SCREEN_H_

#include <string>
#include "base/functional/callback.h"
#include "base/memory/weak_ptr.h"
#include "chrome/browser/ash/login/screens/base_screen.h"
#include "chrome/browser/ui/webui/ash/login/fyde_local_signin_screen_handler.h"

namespace ash {

class FydeLocalSigninScreen
    : public BaseScreen {
  public:
    FydeLocalSigninScreen(base::WeakPtr<FydeLocalSigninView> view,
                          const base::RepeatingClosure& exit_callback);

    ~FydeLocalSigninScreen() override;

    FydeLocalSigninScreen(const FydeLocalSigninScreen&) = delete;
    FydeLocalSigninScreen& operator=(const FydeLocalSigninScreen&) = delete;

  private:
    void HandleCancel();

    // BaseScreen:
    void ShowImpl() override;
    void HideImpl() override;
    void OnUserAction(const base::Value::List& args) override;
    bool HandleAccelerator(LoginAcceleratorAction action) override;

    base::WeakPtr<FydeLocalSigninView> view_;

    base::RepeatingClosure exit_callback_;

    base::WeakPtrFactory<FydeLocalSigninScreen> weak_factory_{this};
};

}

namespace chromeos {
using ::ash::FydeLocalSigninScreen;
}
#endif // ifndef CHROME_BROWSER_ASH_LOGIN_SCREENS_FYDE_LOCAL_SIGNIN_SCREEN_H_
