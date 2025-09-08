// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ash/login/screens/osauth/local_password_setup_screen.h"

#include <string>
#include <utility>

#include "base/check.h"
#include "base/check_op.h"
#include "base/debug/dump_without_crashing.h"
#include "base/functional/bind.h"
#include "base/logging.h"
#include "base/memory/weak_ptr.h"
#include "base/notreached.h"
#include "base/values.h"
#include "chrome/browser/ash/login/oobe_screen.h"
#include "chrome/browser/ash/login/quick_unlock/quick_unlock_factory.h"
#include "chrome/browser/ash/login/screens/base_screen.h"
#include "chrome/browser/ash/login/screens/osauth/base_osauth_setup_screen.h"
#include "chrome/browser/ash/login/wizard_context.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "chrome/browser/ui/webui/ash/login/local_password_setup_handler.h"
#include "chromeos/ash/components/osauth/public/common_types.h"
#include "chromeos/ash/services/auth_factor_config/auth_factor_config.h"
#include "chromeos/ash/services/auth_factor_config/in_process_instances.h"
#include "chromeos/ash/services/auth_factor_config/public/mojom/auth_factor_config.mojom-forward.h"
#include "chromeos/ash/services/auth_factor_config/public/mojom/auth_factor_config.mojom-shared.h"

namespace ash {
namespace {

constexpr const char kUserActionInputPassword[] = "inputPassword";
constexpr const char kUserActionBack[] = "back";

}  // namespace

// static
std::string LocalPasswordSetupScreen::GetResultString(Result result) {
  // LINT.IfChange(UsageMetrics)
  switch (result) {
    case Result::kDone:
      return "Done";
    case Result::kBack:
      return "Back";
    case Result::kNotApplicable:
      return BaseScreen::kNotApplicable;
  }
  // LINT.ThenChange(//tools/metrics/histograms/metadata/oobe/histograms.xml)
}

LocalPasswordSetupScreen::LocalPasswordSetupScreen(
    base::WeakPtr<LocalPasswordSetupView> view,
    const ScreenExitCallback& exit_callback)
    : BaseOSAuthSetupScreen(LocalPasswordSetupView::kScreenId,
                            OobeScreenPriority::DEFAULT),
      view_(std::move(view)),
      exit_callback_(exit_callback) {}

LocalPasswordSetupScreen::~LocalPasswordSetupScreen() = default;

void LocalPasswordSetupScreen::ShowImpl() {
  CHECK(!context()->skip_post_login_screens_for_tests);

  if (!view_) {
    return;
  }
  EstablishKnowledgeFactorGuard(base::BindOnce(
      &LocalPasswordSetupScreen::PreDoShow, weak_factory_.GetWeakPtr()));
}

void LocalPasswordSetupScreen::PreDoShow() {
  if (context()->knowledge_factor_setup.auth_setup_flow != WizardContext::AuthChangeFlow::kInitialSetup) {
    DoShow();
    return;
  }
  InspectContextAndContinue(
      base::BindOnce(&LocalPasswordSetupScreen::InspectContext,
                     weak_factory_.GetWeakPtr()),
      base::BindOnce(&LocalPasswordSetupScreen::AfterContextInspected,
                     weak_factory_.GetWeakPtr()));
}

void LocalPasswordSetupScreen::InspectContext(UserContext* user_context) {
  fyde_local_password_ = std::nullopt;
  if (!user_context) {
    LOG(ERROR) << "Session expired while waiting for user's decision";
    context()->osauth_error = WizardContext::OSAuthErrorKind::kFatal;
    exit_callback_.Run(Result::kNotApplicable);
    return;
  }
  AccountType account_type = user_context->GetAccountId().GetAccountType();
  if (account_type != AccountType::FLINT_ACCOUNT) {
    // only flint account will have local password, and skip DoShow
    return;
  }
  std::optional<LocalPasswordInput> password = user_context->GetFydeLocalPassword();
  if (password) {
    fyde_local_password_ = password->value();
  }
}

void LocalPasswordSetupScreen::AfterContextInspected() {
  if (!fyde_local_password_) {
    DoShow();
    return;
  }
  SetFydeLocalPassword();
}

void LocalPasswordSetupScreen::SetFydeLocalPassword() {
  auth::mojom::PasswordFactorEditor& password_factor_editor =
    auth::GetPasswordFactorEditor(
      quick_unlock::QuickUnlockFactory::GetDelegate(),
      g_browser_process->local_state());

  // skip update mocified_factors, to skip factor_setup_success_screen automatically
  update_modified_factors_ = false;
  password_factor_editor.SetLocalPassword(
    GetToken(), fyde_local_password_.value(),
    base::BindOnce(&LocalPasswordSetupScreen::OnSetLocalPassword,
                   weak_factory_.GetWeakPtr()));
}

void LocalPasswordSetupScreen::DoShow() {
  // The user can go back in the flow, if:
  // 1. They were presented the choice between local vs. online password on the
  //    LocalPasswordSetup screen.
  // 2. They clicked on 'Choose password instead' during main PIN setup.
  bool can_go_back = !context()->knowledge_factor_setup.local_password_forced ||
                     context()->knowledge_factor_setup.pin_setup_mode ==
                         WizardContext::PinSetupMode::kUserChosePasswordInstead;
  bool is_recovery_flow = context()->knowledge_factor_setup.auth_setup_flow ==
                          WizardContext::AuthChangeFlow::kRecovery;
  Profile* profile = ProfileManager::GetPrimaryUserProfile();
  bool is_fyde_profile = profile && profile->IsFydeProfile();
  view_->Show(/*can_go_back=*/can_go_back,
              /*is_recovery_flow=*/is_recovery_flow,
              /*is_fyde_profile*/is_fyde_profile);
}

void LocalPasswordSetupScreen::OnUserAction(const base::Value::List& args) {
  const std::string& action_id = args[0].GetString();
  if (action_id == kUserActionInputPassword) {
    CHECK_EQ(args.size(), 2u);
    const std::string& password = args[1].GetString();
    auth::mojom::PasswordFactorEditor& password_factor_editor =
        auth::GetPasswordFactorEditor(
            quick_unlock::QuickUnlockFactory::GetDelegate(),
            g_browser_process->local_state());
    update_modified_factors_ = true;
    switch (context()->knowledge_factor_setup.auth_setup_flow) {
      case WizardContext::AuthChangeFlow::kInitialSetup:
        password_factor_editor.SetLocalPassword(
            GetToken(), password,
            base::BindOnce(&LocalPasswordSetupScreen::OnSetLocalPassword,
                           weak_factory_.GetWeakPtr()));
        break;
      case WizardContext::AuthChangeFlow::kRecovery:
        password_factor_editor.UpdateOrSetLocalPassword(
            GetToken(), password,
            base::BindOnce(&LocalPasswordSetupScreen::OnUpdateLocalPassword,
                           weak_factory_.GetWeakPtr()));
        break;
      case WizardContext::AuthChangeFlow::kReauthentication:
        NOTREACHED();
    }
    return;
  } else if (action_id == kUserActionBack) {
    exit_callback_.Run(Result::kBack);
    return;
  }
  BaseOSAuthSetupScreen::OnUserAction(args);
}

void LocalPasswordSetupScreen::OnUpdateLocalPassword(
    auth::mojom::ConfigureResult result) {
  if (result != auth::mojom::ConfigureResult::kSuccess) {
    view_->ShowLocalPasswordSetupFailure();
    LOG(ERROR) << "Failed to update local password, error id= "
               << static_cast<int>(result);
    exit_callback_.Run(Result::kDone);
    base::debug::DumpWithoutCrashing();
    return;
  }
  if (update_modified_factors_) {
    context()->knowledge_factor_setup.modified_factors.Put(
        AshAuthFactor::kLocalPassword);
  }
  exit_callback_.Run(Result::kDone);
}

void LocalPasswordSetupScreen::OnSetLocalPassword(
    auth::mojom::ConfigureResult result) {
  if (result != auth::mojom::ConfigureResult::kSuccess) {
    view_->ShowLocalPasswordSetupFailure();
    LOG(ERROR) << "Failed to set local password, error id= "
               << static_cast<int>(result);
    exit_callback_.Run(Result::kDone);
    base::debug::DumpWithoutCrashing();
    return;
  }
  if (update_modified_factors_) {
    context()->knowledge_factor_setup.modified_factors.Put(
      AshAuthFactor::kLocalPassword);
  }
  exit_callback_.Run(Result::kDone);
}

}  // namespace ash
