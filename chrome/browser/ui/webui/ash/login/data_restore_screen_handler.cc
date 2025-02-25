// Copyright 2023 Fyde Innovations. All rights reserved
#include "chrome/browser/ui/webui/ash/login/data_restore_screen_handler.h"
#include "components/login/localized_values_builder.h"
#include "chrome/grit/branded_strings.h"
#include "base/logging.h"

namespace ash {

namespace {
}

DataRestoreScreenHandler::DataRestoreScreenHandler()
    : BaseScreenHandler(kScreenId) {}

DataRestoreScreenHandler::~DataRestoreScreenHandler() = default;

void DataRestoreScreenHandler::SetSystemSalt(const std::string& salt) {
  CallJS("login.DataRestoreScreen.setSystemSalt", salt);
}

void DataRestoreScreenHandler::DeclareLocalizedValues(
    ::login::LocalizedValuesBuilder* builder) {
  builder->Add("dataRestoreDialogSelectFileTitle",
      IDS_DATA_RESTORE_SCREEN_SELECT_FILE_TITLE);
  builder->Add("dataRestoreDialogSelectFileMessage",
      IDS_DATA_RESTORE_SCREEN_SELECT_FILE_MESSAGE);
  builder->Add("dataRestoreDialogSetupBackupFileSelectionLabel",
      IDS_DATA_RESTORE_SCREEN_SETUP_BACKUP_FILE_SELECTION_LABEL);
  builder->Add("dataRestoreDialogSetupInvalidBackupFileMessage",
      IDS_DATA_RESTORE_SCREEN_SETUP_INVALID_BACKUP_FILE_MESSAGE);
  builder->Add("dataRestoreDialogSetupTitle",
      IDS_DATA_RESTORE_SCREEN_SETUP_TITLE);
  builder->Add("dataRestoreDialogSetupSubtitle",
      IDS_DATA_RESTORE_SCREEN_SETUP_SUBTITLE);
  builder->Add("dataRestoreDialogSetupAccountNameLabel",
      IDS_DATA_RESTORE_SCREEN_SETUP_ACCOUNT_NAME_LABEL);
  builder->Add("dataRestoreDialogSetupInvalidAccountNameMessage",
      IDS_DATA_RESTORE_SCREEN_SETUP_INVALID_ACCOUNT_NAME_MESSAGE);
  builder->Add("dataRestoreDialogSetupPasswordLabel",
      IDS_DATA_RESTORE_SCREEN_SETUP_PASSWORD_LABEL);
  builder->Add("dataRestoreDialogSetupInvalidPasswordMessage",
      IDS_DATA_RESTORE_SCREEN_SETUP_INVALID_PASSWORD_MESSAGE);
  builder->Add("dataRestoreDialogSetupNextButton",
      IDS_DATA_RESTORE_SCREEN_SETUP_NEXT_BUTTON);
  builder->Add("dataRestoreDialogInProgressTitle",
      IDS_DATA_RESTORE_SCREEN_IN_PROGRESS_TITLE);
  builder->Add("dataRestoreDialogInProgressSubtitle",
      IDS_DATA_RESTORE_SCREEN_IN_PROGRESS_SUBTITLE);
  builder->Add("dataRestoreDialogErrorTitle",
      IDS_DATA_RESTORE_SCREEN_ERROR_TITLE);
  builder->Add("dataRestoreDialogErrorSubtitle",
      IDS_DATA_RESTORE_SCREEN_ERROR_SUBTITLE);
  builder->Add("dataRestoreDialogErrorViewLogs",
      IDS_DATA_RESTORE_SCREEN_ERROR_VIEW_LOGS);
  builder->Add("dataRestoreDialogSuccessTitle",
      IDS_DATA_RESTORE_SCREEN_SUCCESS_TITLE);
  builder->Add("dataRestoreDialogSuccessSubtitle",
      IDS_DATA_RESTORE_SCREEN_SUCCESS_SUBTITLE);
  builder->Add("dataRestoreDialogServiceLogsTitle",
      IDS_DATA_RESTORE_SCREEN_SERVICE_LOGS_TITLE);
}

void DataRestoreScreenHandler::Show() {
  if (auto* detector = CrosUsbDetector::Get()) {
    cros_usb_device_observation_.Observe(detector);
  } else {
    LOG(WARNING) << "CrosUsbDetector is not available";
  }
  ShowInWebUI();
}

void DataRestoreScreenHandler::Hide() {
  cros_usb_device_observation_.Reset();
}

void DataRestoreScreenHandler::OnUsbDevicesChanged() {
  CrosUsbDetector* detector = CrosUsbDetector::Get();
  DCHECK(detector);
  CallJS("login.DataRestoreScreen.onUsbDevicesChanged");
}

base::WeakPtr<DataRestoreScreenView> DataRestoreScreenHandler::AsWeakPtr() {
  return weak_ptr_factory_.GetWeakPtr();
}

}  // namespace ash
