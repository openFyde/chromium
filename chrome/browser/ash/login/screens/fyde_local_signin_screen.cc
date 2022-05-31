#include "chrome/browser/ash/login/screens/fyde_local_signin_screen.h"

#include "chrome/browser/ui/webui/chromeos/login/fyde_local_signin_screen_handler.h"

namespace ash {
namespace {

constexpr char kUserActionCancel[] = "cancel";

}

FydeLocalSigninScreen::FydeLocalSigninScreen(
    FydeLocalSigninView* view,
    const base::RepeatingClosure& exit_callback)
    : BaseScreen(FydeLocalSigninView::kScreenId,
                 OobeScreenPriority::DEFAULT),
      view_(view),
      exit_callback_(exit_callback) {

    if (view_) {
        view_->Bind(this);
    }
}

FydeLocalSigninScreen::~FydeLocalSigninScreen() {
    if (view_) {
        view_->Unbind();
    }
}

void FydeLocalSigninScreen::OnViewDestroyed(FydeLocalSigninView* view) {
    if (view_ == view) {
        view_ = nullptr;
    }
}

void FydeLocalSigninScreen::ShowImpl() {
  if (!view_)
    return;
  view_->Show();
}

void FydeLocalSigninScreen::HideImpl() {
  if (!view_)
    return;
  view_->Reset();
}

void FydeLocalSigninScreen::OnUserActionDeprecated(const std::string& action_id) {
    if (action_id == kUserActionCancel) {
        HandleCancel();
        return;
    }
    BaseScreen::OnUserActionDeprecated(action_id);
}

void FydeLocalSigninScreen::HandleCancel() {
    view_->Reset();
    exit_callback_.Run();
}

bool FydeLocalSigninScreen::HandleAccelerator(LoginAcceleratorAction action) {
  return false;
}

}
