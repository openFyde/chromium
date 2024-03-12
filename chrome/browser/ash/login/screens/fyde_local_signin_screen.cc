// Copyright 2022 Fyde Innovations. All rights reserved
#include "chrome/browser/ash/login/screens/fyde_local_signin_screen.h"

#include "chrome/browser/ui/webui/ash/login/fyde_local_signin_screen_handler.h"

namespace ash {
namespace {

constexpr char kUserActionCancel[] = "cancel";

}

FydeLocalSigninScreen::FydeLocalSigninScreen(
    base::WeakPtr<FydeLocalSigninView> view,
    const base::RepeatingClosure& exit_callback)
    : BaseScreen(FydeLocalSigninView::kScreenId,
                 OobeScreenPriority::DEFAULT),
      view_(std::move(view)),
      exit_callback_(exit_callback) {}

FydeLocalSigninScreen::~FydeLocalSigninScreen() = default;

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

void FydeLocalSigninScreen::OnUserAction(const base::Value::List& args) {
  const std::string& action_id = args[0].GetString();
  if (action_id == kUserActionCancel) {
    HandleCancel();
    return;
  }
  BaseScreen::OnUserAction(args);
}

void FydeLocalSigninScreen::HandleCancel() {
    view_->Reset();
    exit_callback_.Run();
}

bool FydeLocalSigninScreen::HandleAccelerator(LoginAcceleratorAction action) {
  return false;
}

}  // namespace ash
