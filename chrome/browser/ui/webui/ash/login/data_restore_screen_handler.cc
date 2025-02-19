// Copyright 2023 Fyde Innovations. All rights reserved
#include "chrome/browser/ui/webui/ash/login/data_restore_screen_handler.h"
#include "components/login/localized_values_builder.h"

namespace ash {

namespace {
}

DataRestoreScreenHandler::DataRestoreScreenHandler()
    : BaseScreenHandler(kScreenId) {}

DataRestoreScreenHandler::~DataRestoreScreenHandler() = default;

void DataRestoreScreenHandler::DeclareLocalizedValues(
    ::login::LocalizedValuesBuilder* builder) {}

void DataRestoreScreenHandler::Show() {
  ShowInWebUI();
}

}  // namespace ash
