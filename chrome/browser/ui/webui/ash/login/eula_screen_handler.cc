// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/ash/login/eula_screen_handler.h"

#include <memory>
#include <string>

#include "ash/constants/ash_switches.h"
#include "base/functional/bind.h"
#include "base/command_line.h"
#include "base/strings/stringprintf.h"
#include "base/values.h"
#include "chrome/browser/ash/login/helper.h"
#include "chrome/browser/ash/login/screens/eula_screen.h"
#include "chrome/browser/ui/ash/login/login_display_host.h"
#include "chrome/browser/browser_process.h"
#include "chrome/common/url_constants.h"
#include "chrome/grit/branded_strings.h"
#include "chrome/grit/generated_resources.h"
#include "components/login/localized_values_builder.h"
#include "components/strings/grit/components_strings.h"
#include "rlz/buildflags/buildflags.h"
#include "ui/base/l10n/l10n_util.h"
#include "url/gurl.h"
#include "fydeos/switches/urls/urls_constants.h"

namespace ash {

EulaScreenHandler::EulaScreenHandler() : BaseScreenHandler(kScreenId) {}

EulaScreenHandler::~EulaScreenHandler() = default;

void EulaScreenHandler::Show(const bool is_cloud_ready_update_flow) {
  base::Value::Dict data;
  data.Set("backButtonHidden", is_cloud_ready_update_flow);
  ShowInWebUI(std::move(data));
}

void EulaScreenHandler::Hide() {
}

base::WeakPtr<EulaView> EulaScreenHandler::AsWeakPtr() {
  return weak_ptr_factory_.GetWeakPtr();
}

std::string EulaScreenHandler::GetEulaOnlineUrl() {
  if (base::CommandLine::ForCurrentProcess()->HasSwitch(
          switches::kOobeEulaUrlForTests)) {
    return base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(
        switches::kOobeEulaUrlForTests);
  }

  return fydeos::constants::kOnlineEulaURLPath;
}

std::string EulaScreenHandler::GetPrivacyOnlineUrl() {
  return fydeos::constants::kOnlinePrivacyURLPath;
}

void EulaScreenHandler::DeclareLocalizedValues(
    ::login::LocalizedValuesBuilder* builder) {
  builder->Add("eulaScreenAccessibleTitle", IDS_EULA_SCREEN_ACCESSIBLE_TITLE);
  builder->Add("acceptAgreement", IDS_EULA_ACCEPT_AND_CONTINUE_BUTTON);
  builder->Add("termsOfServiceLoading", IDS_TERMS_OF_SERVICE_SCREEN_LOADING);

  // Online URL to use. May be overridden by tests.
  builder->Add("eulaOnlineUrl", GetEulaOnlineUrl());

  /* MD-OOBE */
  builder->Add("oobeEulaSectionTitle", IDS_OOBE_EULA_SECTION_TITLE);
  builder->Add("oobeEulaIframeLabel", IDS_OOBE_EULA_IFRAME_LABEL);
  builder->Add("oobeEulaAcceptAndContinueButtonText",
               IDS_OOBE_EULA_ACCEPT_AND_CONTINUE_BUTTON_TEXT);

  builder->Add("oobePrivacySectionTitle", IDS_OOBE_PRIVACY_SECTION_TITLE);
  builder->Add("oobePrivacyIframeLabel", IDS_OOBE_PRIVACY_IFRAME_LABEL);

  builder->Add("privacyOnlineUrl", GetPrivacyOnlineUrl());
}

}  // namespace ash
