// Copyright 2020 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fydeos/license/fydeos_license_manager.h"
#include "base/values.h"
#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "base/files/file_util.h"
#include "base/task/thread_pool.h"
#include "chrome/browser/ash/login/oobe_configuration.h"
#include "chromeos/ash/components/network/portal_detector/network_portal_detector.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "chrome/browser/ash/policy/enrollment/enrollment_token_provider.h"
#include "components/prefs/pref_service.h"
#include "chromeos/ash/components/network/network_state_handler.h"
#include "fydeos/chromeos/ash/components/dbus/fydeos_shell_client/fydeos_shell_client.h"
#include "fydeos/chromeos/ash/components/dbus/fydeos_shell_client/shell_state.h"
#include "fydeos/license/fydeos_callback_status.h"
#include "fydeos/switches/license/license_constants.h"
#include "fydeos/license/fydeos_license_enforcement.h"

#include "fydeos/switches/license/license_switches.h"
#include "fydeos/switches/account/account_switches.h"
#include "base/strings/string_number_conversions.h"
#include "fydeos/constants/fydeos_constants.h"
#include "fydeos/prefs/fydeos_pref_names.h"

using FydeOSShellClient = fydeos::ash::FydeOSShellClient;
using ShellState = fydeos::ash::ShellState;
using LicenseStateType = fydeos::constants::LicenseStateType;
using LicenseEnforcementLevel = fydeos::constants::LicenseEnforcementLevel;

namespace fydeos::license {
namespace  {
  const char kShellCmd[] = "/usr/share/fydeos_shell/license-utils.sh %s";
  const char kParamId[] = "id";
  const char kParamReadLicense[] = "read";
  const char kParamWriteLicense[] = "write";
  const char *kLicenseAttrs[] = {"expire_date", "license_type", "license", "expiration_action", "show_license_in_settings", "log_out_interval"};
  const char kVpdCmd[] = "vpd -g %s";
  const char kParamSerialNumber[] = "serial_number";
  const int kTestLicenseCheckRepeatTimeInMinutes = 1;
  const int kLicenseCheckRepeatTimeInMinutes = 30;
  const int kLicenseCheckDelayTimeInSeconds = 30;
  LicenseManager* g_license_manager = nullptr;

  FydeOSShellClient* GetShellClient() {
    return FydeOSShellClient::Get();
  }

  std::string GetIDCmd() {
    return base::StringPrintf(kShellCmd, kParamId);
  }

  std::string GetSerialNumberCmd() {
    return base::StringPrintf(kVpdCmd, kParamSerialNumber);
  }

  std::string GetLicenseCmd() {
    return base::StringPrintf(kShellCmd, kParamReadLicense);
  }

  std::string SaveLicenseCmd(base::Value::Dict* license) {
    std::string result(kParamWriteLicense);
    const std::string* value;
    std::optional<int> int_value;
    for (auto & kLicenseAttr : kLicenseAttrs) {
      if ((value = license->FindString(kLicenseAttr))) {
        result.append(" ").append(kLicenseAttr).append("=").append(*value);
      } else if ((int_value = license->FindInt(kLicenseAttr))) {
        result.append(" ").append(kLicenseAttr).append("=").append(base::NumberToString(int_value.value()));
      }
    }
    return base::StringPrintf(kShellCmd, result.c_str());
  }
}  // namespace

using fydeos::constants::LicenseEnforcementLevel;

void LicenseManager::Initialize() {
  CHECK(g_license_manager == nullptr);
  g_license_manager = new LicenseManager();
}


LicenseManager* LicenseManager::Get() {
  return g_license_manager;
}

void LicenseManager::Shutdown() {
  CHECK(g_license_manager != nullptr);
  g_license_manager->Stop();
  delete g_license_manager;
  g_license_manager = nullptr;
}

LicenseManager::LicenseManager()
  : mode_(FetchMode::OfflineMode),
    init_timer_(std::make_unique<base::OneShotTimer>()),
    timer_(std::make_unique<base::RepeatingTimer>()),
    online_fetcher_(std::make_unique<LicenseOnlineFetcher>()),
    validator_(std::make_unique<LicenseValidator>()),
    enforcement_(std::make_unique<LicenseEnforcement>()) {
  if (::ash::LoginState::IsInitialized())
    ::ash::LoginState::Get()->AddObserver(this);
  VLOG(1) << "LicenseManager inited.";
}

LicenseManager::~LicenseManager() = default;

void LicenseManager::Start() {
  IGetId();
  IGetSerialNumber();
  if (!init_timer_->IsRunning()) {
    init_timer_->Start(
    FROM_HERE,
    base::Seconds(kLicenseCheckDelayTimeInSeconds),
    base::BindOnce(&LicenseManager::InitCheckLicense, base::Unretained(this)));
  }
  if (!timer_->IsRunning()) {
    VLOG(1) << "Start timer of checking.";
    timer_->Start(
    FROM_HERE,
    base::Minutes(fydeos::switches::IsLicenseTestMode() ?
      kTestLicenseCheckRepeatTimeInMinutes : kLicenseCheckRepeatTimeInMinutes),
    base::BindRepeating(&LicenseManager::CheckLicense, weak_ptr_factory_.GetWeakPtr()));
  }
}

void LicenseManager::Stop() {
  if (init_timer_->IsRunning())
    init_timer_->Stop();
  if (timer_->IsRunning())
    timer_->Stop();
  Reset();
}

void LicenseManager::InitCheckLicense() {
  is_init_check_ = true;
  CheckLicenseInternal();
}

 void LicenseManager::CheckLicense() {
  is_init_check_ = false;
  CheckLicenseInternal();
}


void LicenseManager::CheckLicenseInternal() {
  if (is_valid_ && !id_.empty()) {
    VLOG(1) << "Everything is fine, stop checking.";
    init_timer_->Stop();
    timer_->Stop();
    return;
  }
  if (id_.empty()) {
    LOG(ERROR) << "There is no license id, invalid system.";
    OnInvalid();
    return;
  }
  if (state_ != ManagerState::Idle) {
    LOG(WARNING) << "There is an other thread running.";
    return;
  }
  state_ = ManagerState::Running;
  if (mode_ == FetchMode::OfflineMode) {
    VLOG(1) << "fetch offline license";
    IFetchOfflineLicense();
  } else {
    VLOG(1) << "fetch online license";
    IFetchOnlineLicense();
  }
}

void LicenseManager::IGetId() {
  GetShellClient()->SyncExec(GetIDCmd(),
    base::BindOnce(&LicenseManager::OnGotId, weak_ptr_factory_.GetWeakPtr()));
}

void LicenseManager::OnGotId(std::optional<ShellState> state) {
  if (!state || state->code != 0) {
    LOG(ERROR) << "Can't get license id.\n";
  } else {
    VLOG(1) << "Get id:" << state->result;
    id_ = state->result;
    id_.erase(std::remove(id_.begin(), id_.end(), '\n'), id_.end());
  }
}

void LicenseManager::IGetOEMToken() {
  auto* oobe_configuration = ::ash::OobeConfiguration::Get();
  auto token = policy::GetFydeEnrollmentToken(oobe_configuration);
  if (token.has_value()) {
    OnGotOEMToken(token.value());
  }
}

void LicenseManager::OnGotOEMToken(const std::string& token) {
  if (!token.empty()) {
    VLOG(1) << "Get OEM token:" << token;
    oem_token_ = token;
  }
}

void LicenseManager::IGetSerialNumber() {
  GetShellClient()->SyncExec(GetSerialNumberCmd(),
    base::BindOnce(&LicenseManager::OnGotSerialNumber, weak_ptr_factory_.GetWeakPtr()));
}

void LicenseManager::OnGotSerialNumber(std::optional<ShellState> state) {
  if (!state || state->code != 0) {
    LOG(ERROR) << "Can't get serial_number.\n";
  } else {
    VLOG(1) << "Get serial_number:" << state->result;
    serial_number_ = state->result;
    serial_number_.erase(
        std::remove(serial_number_.begin(), serial_number_.end(), '\n'),
        serial_number_.end());
  }
  IGetOEMToken();
}

void LicenseManager::OnFetchError(int errCode, const std::string& errMsg) {
  if (mode_ == FetchMode::OfflineMode) {
    return;
  }
  LOG(ERROR) << "Fetch online license verification error";
  // do nothing;
}

void LicenseManager::OnValidateError(int errCode, const std::string& errMsg, std::optional<base::Value> license) {
  if (mode_ == FetchMode::OnlineMode && errCode == -9 && license) {
    IStoreLicense(std::move(license));
  }
  OnError(errCode, errMsg);
}

void LicenseManager::OnError(int errCode, const std::string& errMsg) {
  if (mode_ == FetchMode::OfflineMode) {
    LOG(WARNING) << "Error occured from offline verification, code:"
      << errCode << " message:" << errMsg << " Switch to online verification.";
    is_new_license_ = (errCode == -3); // no license field
    mode_ = FetchMode::OnlineMode;
    IFetchOnlineLicense();
  } else {
    LOG(ERROR) << "Error occured from online verification, code:"
      << errCode << " message:" << errMsg;
    OnInvalid();
  }
}

void LicenseManager::OnValid(std::optional<base::Value> license) {
  VLOG(1) << "license is valid.";
  if (mode_ == FetchMode::OfflineMode && is_init_check_) {
    ISaveLocalLicenseString(std::move(license));
    mode_ = FetchMode::OnlineMode;
    IFetchOnlineLicense();
    return;
  }
  is_valid_ = true;
  state_ = ManagerState::Idle;
  init_timer_->Stop();
  timer_->Stop();
  if (mode_ == FetchMode::OfflineMode) {
    // save string, to prevent call shellclient to store the same license get from online
    ISaveLocalLicenseString(std::move(license));
    // always fetch online
    mode_ = FetchMode::OnlineMode;
    IFetchOnlineLicense();
  } else if (mode_ == FetchMode::OnlineMode) {
    IStoreLicense(std::move(license));
  }
  Reset();
}

void LicenseManager::Reset() {
  // restore punishment
  VLOG(1) << "Reset invalid effect.";
  NotifyObservers();
  enforcement_->StopEnforcement();
}

bool LicenseManager::ShouldStartEnforcement() const {
  return !is_init_check_;
}

void LicenseManager::OnInvalid() {
  // trigger punishment
  is_valid_ = false;
  state_ = ManagerState::Idle;
  mode_ = FetchMode::OfflineMode;
  VLOG(1) << "Start invalid effect.";
  NotifyObservers();
  if (!ShouldStartEnforcement()) {
    VLOG(2) << "Skip enforcement.";
    return;
  }
  StartEnforcement();
}

void LicenseManager::StartEnforcement() {
  if (!profile_) {
    profile_ = g_browser_process->profile_manager()->GetActiveUserProfile();
  }
  if (!profile_ || !profile_->GetPrefs()) return;
  PrefService* prefs = g_browser_process->local_state();
  if (!prefs) return;
  LicenseEnforcementLevel level = static_cast<LicenseEnforcementLevel>(prefs->GetInteger(fydeos::prefs::kFydeLicenseEnforcementLevel));
  EnforcementMode mode;
  switch (level) {
    case LicenseEnforcementLevel::kNone:
      mode = EnforcementModeLevel0;
      break;
    case LicenseEnforcementLevel::kForcePopup:
      mode = EnforcementModeLevel1;
      break;
    case LicenseEnforcementLevel::kForceQuit:
      mode = EnforcementModeLevel2;
      break;
    default:
      mode = EnforcementModeLevel2;
      break;
  }
  int log_out_interval = prefs->GetInteger(fydeos::prefs::kFydeLicenseEnforcementLogOutInterval);
  enforcement_->StartEnforcement(profile_, id_, serial_number_, mode, log_out_interval);
}

void LicenseManager::SkipOnline() {
  state_ = ManagerState::Idle;
  mode_ = FetchMode::OfflineMode;
}

void LicenseManager::IValidateLicense(std::optional<std::string> license) {
  if (license.has_value())
    VLOG(1) << "Start validating." << license.value();
  validator_->StartValidate(id_,
                            std::move(license),
                            base::BindOnce(
                              &LicenseManager::OnValid,
                              weak_ptr_factory_.GetWeakPtr()),
    base::BindOnce(&LicenseManager::OnValidPref, weak_ptr_factory_.GetWeakPtr()),
    base::BindOnce(&LicenseManager::OnValidateError, weak_ptr_factory_.GetWeakPtr()));
}

void LicenseManager::OnValidPref(int licenseType, bool expired, int expirationAction, int showLicenseInSettings, int logOutInterval) {
  if (!profile_) {
    profile_ = g_browser_process->profile_manager()->GetActiveUserProfile();
  }
  if (!profile_ || !profile_->GetPrefs()) return;

  PrefService* prefs = g_browser_process->local_state();
  if (!prefs) return;

  LicenseStateType state = LicenseStateType::kUnspecified;
  switch (licenseType) {
    // trial
    case 0:
      state = expired ? LicenseStateType::kLicenseForYouExpired : LicenseStateType::kLicenseForYouTrial;
      break;
    // valid
    case 1:
      state = expired ? LicenseStateType::kLicenseForYouExpired : LicenseStateType::kLicenseForYouValid;
      break;
    // enterprise trial
    case 2:
      state = expired ? LicenseStateType::kLicenseForEnterpriseExpired : LicenseStateType::kLicenseForEnterpriseTrial;
      break;
    // enterprise valid
    case 3:
      state = expired ? LicenseStateType::kLicenseForEnterpriseExpired : LicenseStateType::kLicenseForEnterpriseValid;
      break;
    // unlicensed
    case 4:
      state = LicenseStateType::kUnlicensed;
      break;
    default:
      state = LicenseStateType::kUnspecified;
      break;
  }
  prefs->SetInteger(fydeos::prefs::kFydeLicenseStateType, static_cast<int>(state));

  VLOG(2) << "license pref, state: " << static_cast<int>(state)
    << "expiration action: " << expirationAction
    << " show license in settings: " << showLicenseInSettings
    << " log out interval: " << logOutInterval;

  LicenseEnforcementLevel level = LicenseEnforcementLevel::kNone;
  switch (expirationAction) {
    case 0:
      level = LicenseEnforcementLevel::kNone;
      break;
    case 1:
      level = LicenseEnforcementLevel::kForcePopup;
      break;
    case 2:
      level = LicenseEnforcementLevel::kForceQuit;
      break;
    default:
      level = LicenseEnforcementLevel::kNone;
      break;
  }
  prefs->SetInteger(fydeos::prefs::kFydeLicenseEnforcementLevel, static_cast<int>(level));

  prefs->SetBoolean(fydeos::prefs::kFydeLicenseShouldShowInSettings, showLicenseInSettings == 1);
  prefs->SetInteger(fydeos::prefs::kFydeLicenseEnforcementLogOutInterval, logOutInterval);
}

void LicenseManager::ISaveLocalLicenseString(std::optional<base::Value> license) {
  base::Value::Dict *licenseDict = license->GetIfDict();
  if (!licenseDict) return;
  const std::string* value;
  if ((value = licenseDict->FindString("license"))) {
    license_string = *value;
  } else {
    license_string.clear();
  }
}

void LicenseManager::IStoreLicense(std::optional<base::Value> license) {
  VLOG(1) << "Store license";
  base::Value::Dict *licenseDict = license->GetIfDict();
  const std::string* value;
  if (licenseDict) {
    if ((value = licenseDict->FindString("license")) && *value == license_string) {
      VLOG(1) << "License is not changed, skip store.";
      return;
    }
    GetShellClient()->SyncExec(SaveLicenseCmd(licenseDict),
      base::BindOnce(&LicenseManager::OnStoredLicense,
        weak_ptr_factory_.GetWeakPtr()));
  }
}

void LicenseManager::OnStoredLicense(std::optional<ShellState> state) {
  if ( state && (state->code != 0))
    LOG(WARNING) << "Store license got error:" << state->result;
  else
    VLOG(1) << "Store license successfully";
}

void LicenseManager::IFetchOfflineLicense() {
  GetShellClient()->SyncExec(GetLicenseCmd(),
    base::BindOnce(
      &LicenseManager::OnFetchedOfflineLicense, weak_ptr_factory_.GetWeakPtr()));
}

void LicenseManager::OnFetchedOfflineLicense(std::optional<ShellState> state) {
  if (!state) {
    OnError(-1, "No shell result.");
  } else {
    VLOG(1) << "Get offline license:" << state->result;
    if (state->code == 0)
      IValidateLicense(state->result);
    else
      OnError(state->code, state->result);
  }
}

void LicenseManager::LoggedInStateChanged() {
  if (::ash::LoginState::Get()->IsUserLoggedIn() || ::ash::LoginState::Get()->IsKioskSession())
    Start();
  else
    Stop();
}

void LicenseManager::IFetchOnlineLicense() {
  ::ash::NetworkStateHandler* network_state_handler =
      ::ash::NetworkHandler::Get()->network_state_handler();
  if (!network_state_handler) return;
  const ::ash::NetworkState* network = network_state_handler->DefaultNetwork();
  if (!network) return;
  ::ash::NetworkState::PortalState portalStatus = network->portal_state();
  if (portalStatus !=
      ::ash::NetworkState::PortalState::kOnline) {
    VLOG(1) << "Not online currently, skip fetching online license";
    SkipOnline();
    return;
  }
  const std::string basic_token = fydeos::switches::GetFydeBasicLicenseToken();
  online_fetcher_->StartFetch(id_,
                              serial_number_,
                              is_new_license_,
                              oem_token_,
                              basic_token,
                              base::BindOnce(
                                &LicenseManager::IValidateLicense,
                                weak_ptr_factory_.GetWeakPtr()),
    base::BindOnce(&LicenseManager::OnFetchError, weak_ptr_factory_.GetWeakPtr()));
}

void LicenseManager::AddObserver(Observer* observer) {
  observer_list_.AddObserver(observer);
}

void LicenseManager::RemoveObserver(Observer* observer) {
  observer_list_.RemoveObserver(observer);
}

void LicenseManager::NotifyObservers() {
  for (auto& observer : observer_list_)
    observer.OnLicenseStateChanged(is_valid_);
}

}  // namespace fydeos::license
