#ifndef CHROME_BROWSER_UI_WEBUI_CHROMEOS_LOGIN_FYDE_LOCAL_SIGNIN_SCREEN_HANDLER_H_
#define CHROME_BROWSER_UI_WEBUI_CHROMEOS_LOGIN_FYDE_LOCAL_SIGNIN_SCREEN_HANDLER_H_

#include <string>
#include "chrome/browser/ui/webui/chromeos/login/base_screen_handler.h"

namespace ash {
class FydeLocalSigninScreen;
class Key;
}

namespace chromeos {


class FydeLocalSigninView {
  public:
    constexpr static StaticOobeScreenId kScreenId{"fyde-local-signin"};

    virtual ~FydeLocalSigninView() = default;

    // Shows the contents of the screen.
    virtual void Show() = 0;

    // Binds `screen` to the view.
    virtual void Bind(ash::FydeLocalSigninScreen* screen) = 0;

    // Unbinds the screen from the view.
    virtual void Unbind() = 0;

    // Clear the input fields on the screen.
    virtual void Reset() = 0;

    // Set error state.
    virtual void SetErrorState(const std::string& username, int errorState) = 0;
};

class FydeLocalSigninScreenHandler : public FydeLocalSigninView,
                                     public BaseScreenHandler {
  public:
    using TView = FydeLocalSigninView;

    explicit FydeLocalSigninScreenHandler();
    ~FydeLocalSigninScreenHandler() override;

    FydeLocalSigninScreenHandler(const FydeLocalSigninScreenHandler&) =
      delete;
    FydeLocalSigninScreenHandler& operator=(
        const FydeLocalSigninScreenHandler&) = delete;

  private:
    void HandleCompleteAuth(const std::string& username,
                            const std::string& password);

    void DoCompleteLogin(const std::string& username,
                         const ash::Key& key);

    void Show() override;
    void Bind(ash::FydeLocalSigninScreen* screen) override;
    void Unbind() override;
    void Reset() override;
    void SetErrorState(const std::string& username, int errorState) override;

    void RegisterMessages() override;
    void DeclareLocalizedValues(
        ::login::LocalizedValuesBuilder* builder) override;
    void InitializeDeprecated() override;

    ash::FydeLocalSigninScreen* screen_ = nullptr;

    bool show_on_init_ = false;
};

}

namespace ash {
using ::chromeos::FydeLocalSigninScreenHandler;
using ::chromeos::FydeLocalSigninView;
}
#endif // ifndef CHROME_BROWSER_UI_WEBUI_CHROMEOS_LOGIN_FYDE_LOCAL_SIGNIN_SCREEN_HANDLER_H_
