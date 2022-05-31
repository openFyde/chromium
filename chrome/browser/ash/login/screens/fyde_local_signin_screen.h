#ifndef CHROME_BROWSER_ASH_LOGIN_SCREENS_FYDE_LOCAL_SIGNIN_SCREEN_H_
#define CHROME_BROWSER_ASH_LOGIN_SCREENS_FYDE_LOCAL_SIGNIN_SCREEN_H_

#include <string>
#include "base/callback.h"
#include "chrome/browser/ash/login/screens/base_screen.h"
#include "chrome/browser/ui/webui/chromeos/login/fyde_local_signin_screen_handler.h"

namespace ash {

class FydeLocalSigninScreen
    : public BaseScreen {
  public:
    FydeLocalSigninScreen(FydeLocalSigninView* view,
                          const base::RepeatingClosure& exit_callback);

    ~FydeLocalSigninScreen() override;

    FydeLocalSigninScreen(const FydeLocalSigninScreen&) = delete;
    FydeLocalSigninScreen& operator=(const FydeLocalSigninScreen&) = delete;

    void OnViewDestroyed(FydeLocalSigninView* view);

  private:
    void HandleCancel();

    // BaseScreen:
    void ShowImpl() override;
    void HideImpl() override;
    void OnUserActionDeprecated(const std::string& action_id) override;
    bool HandleAccelerator(LoginAcceleratorAction action) override;

    FydeLocalSigninView* view_ = nullptr;

    base::RepeatingClosure exit_callback_;

    base::WeakPtrFactory<FydeLocalSigninScreen> weak_factory_{this};
};

}

namespace chromeos {
using ::ash::FydeLocalSigninScreen;
}
#endif // ifndef CHROME_BROWSER_ASH_LOGIN_SCREENS_FYDE_LOCAL_SIGNIN_SCREEN_H_
