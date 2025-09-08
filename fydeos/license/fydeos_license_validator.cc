// Copyright 2020 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fydeos/license/fydeos_license_validator.h"
#include "base/logging.h"
#include "base/containers/span.h"
#include "base/strings/stringprintf.h"
#include "base/time/time.h"
#include "base/base64.h"
#include "base/json/json_reader.h"
#include "fydeos/switches/account/policy_constants.h"

namespace fydeos::license {
namespace  {
  const char kExpireDate[]="expire_date";
  const char kLicenseType[]="license_type";
  const char kLicense[]="license";
  const int kMaxType = 128;

  const char kExpirationAction[]="expiration_action";
  const char kShowLicenseInSettings[] = "show_license_in_settings";
  const char kLogOutInterval[] = "log_out_interval";
}  // namespace

LicenseValidator::LicenseValidator() = default;
LicenseValidator::~LicenseValidator() = default;

void LicenseValidator::StartValidate(const std::string& id,
                                     std::optional<std::string> license,
    SuccessCallback<std::optional<base::Value>> success_callback,
    SavePrefCallback save_pref_callback,
    ErrorWithSaveCallback<std::optional<base::Value>> err_callback) {
  if (id.empty() || license->empty()) {
    std::move(err_callback).Run(-1, "Id or license is empty.", std::nullopt);
    return;
  }
  std::optional<base::Value> json = base::JSONReader::Read(*license);
  base::Value::Dict* license_dict = nullptr;
  if (!json.has_value() || !(license_dict = json->GetIfDict())) {
    std::move(err_callback).Run(-2, "license parse error", std::nullopt);
    return;
  }
  const std::string* signed_token;
  if (!(signed_token = license_dict->FindString(kLicense)) || signed_token->empty()) {
    std::move(err_callback).Run(-3, "No license found.", std::nullopt);
    return;
  }
  std::string signature;
  if (!base::Base64Decode(*signed_token, &signature)) {
    std::move(err_callback).Run(-11, "signature decode error.", std::nullopt);
    return;
  }
  const std::string* expired_date;
  if (!(expired_date = license_dict->FindString(kExpireDate))) {
    std::move(err_callback).Run(-4, "No expire date found.", std::nullopt);
    return;
  }
  const std::string* license_type;
  if (!(license_type = license_dict->FindString(kLicenseType))) {
    std::move(err_callback).Run(-5, "No license type found.", std::nullopt);
    return;
  }
  std::optional<int> expiration_action;
  if (!(expiration_action = license_dict->FindInt(kExpirationAction))) {
    std::move(err_callback).Run(-11, "No expiration action found.", std::nullopt);
    return;
  }
  std::optional<int> show_license_in_settings;
  if (!(show_license_in_settings = license_dict->FindInt(kShowLicenseInSettings))) {
    std::move(err_callback).Run(-12, "No show_license_in_settings found.", std::nullopt);
    return;
  }
  std::optional<int> log_out_interval;
  if (!(log_out_interval = license_dict->FindInt(kLogOutInterval))) {
    std::move(err_callback).Run(-13, "No log_out_interval found.", std::nullopt);
    return;
  }
  VLOG(2) << "id:" << id << " license_type:" << *license_type
    << " expiration_action: " << expiration_action.value()
    << " show_license_in_settings: " << show_license_in_settings.value()
    << " log_out_interval: " << log_out_interval.value()
    << " expired_date:" << *expired_date << " token:" << *signed_token;
  int lType = std::atoi(license_type->c_str());
  if (lType < 0 || lType > kMaxType) {
    std::move(err_callback).Run(-10, "license type error.", std::nullopt);
    return;
  }
  const std::string key = std::string(
    reinterpret_cast<const char*>(fydeos::constants::kFydeOSPolicyVerificationKey),
    fydeos::constants::kFydeOSPolicyVerificationKeyLength);
  if (!signature_verifier_.VerifyInit(
        crypto::SignatureVerifier::RSA_PKCS1_SHA256,
    base::as_bytes(base::span(signature)),
    base::as_bytes(base::span(key)))) {
    std::move(err_callback).Run(-6, "verifier init error", std::nullopt);
    return;
  }
  std::string signed_data(
    base::StringPrintf("%s.%s.%s.%d.%d.%d",
      id.c_str(), license_type->c_str(), expired_date->c_str(),
      expiration_action.value(), show_license_in_settings.value(), log_out_interval.value()));
  signature_verifier_.VerifyUpdate(
      base::as_bytes(base::span(signed_data)));
  if (!signature_verifier_.VerifyFinal()) {
    std::move(err_callback).Run(-7, "invalid license.", std::nullopt);
    return;
  }
  // unlicensed (4)
  if (lType != 4) {
    base::Time expired_time;
    if (!base::Time::FromUTCString(expired_date->c_str(), &expired_time)) {
      std::move(err_callback).Run(-8, "invalid expired date.", std::nullopt);
      return;
    }
    VLOG(2) << "Now:" << base::Time::NowFromSystemTime()
            << " expired_time:" << expired_time;
    if (base::Time::NowFromSystemTime() > expired_time) {
      std::move(save_pref_callback).Run(lType, true, expiration_action.value(), show_license_in_settings.value(), log_out_interval.value());
      std::move(err_callback).Run(-9, "your license is expired.", std::move(json));
      return;
    }
  }
  std::move(save_pref_callback).Run(lType, false, expiration_action.value(), show_license_in_settings.value(), log_out_interval.value());
  std::move(success_callback).Run(std::move(json));
}

}  // namespace fydeos::license
