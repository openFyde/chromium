// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ash/login/screens/eula_screen.h"

#include "ash/constants/ash_features.h"
#include "ash/constants/ash_switches.h"
#include "base/functional/bind.h"
#include "base/functional/callback_helpers.h"
#include "base/check.h"
#include "base/metrics/histogram_functions.h"
#include "base/notreached.h"
#include "chrome/browser/ash/customization/customization_document.h"
#include "chrome/browser/ash/login/startup_utils.h"
#include "chrome/browser/ash/login/wizard_context.h"
#include "chrome/browser/ash/login/wizard_controller.h"
#include "chrome/browser/ash/policy/enrollment/enrollment_requisition_manager.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/ui/webui/ash/login/eula_screen_handler.h"
#include "chromeos/ash/components/dbus/dbus_thread_manager.h"
#include "chromeos/dbus/common/dbus_callback.h"
#include "chromeos/dbus/tpm_manager/tpm_manager.pb.h"
#include "chromeos/dbus/tpm_manager/tpm_manager_client.h"
#include "fydeos/switches/misc/misc_switches.h"

namespace ash {
namespace {

constexpr const char kUserActionAcceptButtonClicked[] = "accept-button";
constexpr const char kUserActionBackButtonClicked[] = "back-button";

struct EulaUserAction {
  const char* name_;
  EulaScreen::UserAction uma_name_;
};

const EulaUserAction actions[] = {
    {kUserActionAcceptButtonClicked,
     EulaScreen::UserAction::kAcceptButtonClicked},
    {kUserActionBackButtonClicked, EulaScreen::UserAction::kBackButtonClicked},
};

void RecordEulaScreenAction(EulaScreen::UserAction value) {
  base::UmaHistogramEnumeration("OOBE.EulaScreen.UserActions", value);
}

bool IsEulaUserAction(const std::string& action_id) {
  for (const auto& el : actions) {
    if (action_id == el.name_)
      return true;
  }
  return false;
}

void RecordUserAction(const std::string& action_id) {
  for (const auto& el : actions) {
    if (action_id == el.name_) {
      RecordEulaScreenAction(el.uma_name_);
      return;
    }
  }
  NOTREACHED() << "Unexpected action id: " << action_id;
}

}  // namespace

// static
std::string EulaScreen::GetResultString(Result result) {
  switch (result) {
    case Result::ACCEPTED:
      return "Accepted";
    case Result::BACK:
    case Result::ALREADY_ACCEPTED:
    case Result::NOT_APPLICABLE:
      return BaseScreen::kNotApplicable;
  }
}

EulaScreen::EulaScreen(base::WeakPtr<EulaView> view,
                       const ScreenExitCallback& exit_callback)
    : BaseScreen(EulaView::kScreenId, OobeScreenPriority::DEFAULT),
      view_(std::move(view)),
      exit_callback_(exit_callback) {
  DCHECK(view_);
}

EulaScreen::~EulaScreen() = default;

bool EulaScreen::MaybeSkip(WizardContext& context) {
  if (policy::EnrollmentConfig::IsZeroTouchEnrollmentFydeForced()) {
    exit_callback_.Run(Result::NOT_APPLICABLE);
    return true;
  }
  const auto* const demo_setup_controller =
      WizardController::default_controller()->demo_setup_controller();
  if (demo_setup_controller) {
    exit_callback_.Run(Result::NOT_APPLICABLE);
    return true;
  }
  // This should be kept in sync with `testapi_shouldSkipEula`. If the logic
  // became too complicated we need to consider extract and reuse parts of it.

  if (!context.is_branded_build && !fydeos::switches::IsFydeCustomEnabled()) {
    exit_callback_.Run(Result::NOT_APPLICABLE);
    return true;
  }

  if (StartupUtils::IsEulaAccepted() && !context.is_cloud_ready_update_flow) {
    exit_callback_.Run(Result::ALREADY_ACCEPTED);
    return true;
  }

  // Remora (CfM) devices are enterprise only. To enroll device it is required
  // to accept ToS on the server side. Thus for such devices it is not needed to
  // accept EULA on the client side.
  // TODO(https://crbug.com/1190897): Refactor to use `context` instead.
  if (policy::EnrollmentRequisitionManager::IsRemoraRequisition()) {
    exit_callback_.Run(Result::NOT_APPLICABLE);
    return true;
  }

  return false;
}

void EulaScreen::ShowImpl() {
  view_->Show(context()->is_cloud_ready_update_flow);
}

void EulaScreen::HideImpl() {
  if (view_)
    view_->Hide();
}

void EulaScreen::OnUserAction(const base::Value::List& args) {
  const std::string& action_id = args[0].GetString();
  if (!IsEulaUserAction(action_id)) {
    BaseScreen::OnUserAction(args);
    return;
  }
  RecordUserAction(action_id);
  if (action_id == kUserActionAcceptButtonClicked) {
    exit_callback_.Run(Result::ACCEPTED);
  } else if (action_id == kUserActionBackButtonClicked) {
    exit_callback_.Run(Result::BACK);
  }
}

}  // namespace ash
