// Copyright 2023 Fyde Innovations. All rights reserved
#include "chrome/browser/ash/login/screens/data_restore_screen.h"
#include "chrome/browser/ui/webui/ash/login/data_restore_screen_handler.h"
#include "base/logging.h"


namespace ash {

namespace {
}

DataRestoreScreen::DataRestoreScreen(
    base::WeakPtr<DataRestoreScreenView> view,
    const base::RepeatingClosure& exit_callback)
    : BaseScreen(DataRestoreScreenView::kScreenId, OobeScreenPriority::DEFAULT),
      view_(std::move(view)),
      exit_callback_(exit_callback) {}

DataRestoreScreen::~DataRestoreScreen() = default;

void DataRestoreScreen::ShowImpl() {
  if (view_) {
    view_->Show();
  }
}

void DataRestoreScreen::HideImpl() {}

void DataRestoreScreen::OnUserAction(const base::Value::List& args) {
  const std::string& action_id = args[0].GetString();
  VLOG(1) << "DataRestoreScreen::OnUserAction: " << action_id;
}

}  // namespace ash
