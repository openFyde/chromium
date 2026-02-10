// Copyright 2023 Fyde Innovations. All rights reserved
#include "chrome/browser/ash/login/screens/data_restore_screen.h"
#include "chrome/browser/ui/webui/ash/login/data_restore_screen_handler.h"
#include "base/logging.h"
#include "chromeos/ash/components/cryptohome/system_salt_getter.h"


namespace ash {

namespace {
constexpr const char kUserActionExitClicked[] = "data-restore-exit";
}

DataRestoreScreen::DataRestoreScreen(
    base::WeakPtr<DataRestoreScreenView> view,
    const base::RepeatingClosure& exit_callback)
    : BaseScreen(DataRestoreScreenView::kScreenId, OobeScreenPriority::DEFAULT),
      view_(std::move(view)),
      exit_callback_(exit_callback) {
  SystemSaltGetter::Get()->GetSystemSalt(
      base::BindOnce(&DataRestoreScreen::OnGetSystemSalt,
                     weak_ptr_factory_.GetWeakPtr()));
}

DataRestoreScreen::~DataRestoreScreen() = default;

void DataRestoreScreen::OnGetSystemSalt(const std::string& salt) {
  if (view_) {
    view_->SetSystemSalt(salt);
  }
}

void DataRestoreScreen::ShowImpl() {
  if (view_) {
    view_->Show();
  }
}

void DataRestoreScreen::HideImpl() {
  if (view_) {
    view_->Hide();
  }
}

void DataRestoreScreen::OnUserAction(const base::Value::List& args) {
  const std::string& action_id = args[0].GetString();
  if (action_id == kUserActionExitClicked) {
    exit_callback_.Run();
  } else {
    BaseScreen::OnUserAction(args);
  }
}

}  // namespace ash
