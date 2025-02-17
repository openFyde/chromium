#include "fydeos/misc/fydeos_statistics_collector.h"
#include "fydeos/switches/services/services_switches.h"
#include "fydeos/switches/services/services_constants.h"
#include "chrome/browser/profiles/profile.h"
#include "chromeos/ash/components/system/statistics_provider.h"
#include "base/system/sys_info.h"
#include "chrome/browser/browser_process.h"
#include "chrome/common/channel_info.h"
#include "components/version_info/version_info.h"
#include "chromeos/version/version_loader.h"
#include "chromeos/ash/components/dbus/dbus_thread_manager.h"
#include "fydeos/chromeos/ash/components/dbus/fydeos_shell_client/fydeos_shell_client.h"
#include "chrome/browser/ash/profiles/profile_helper.h"
#include "components/user_manager/user.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "components/prefs/pref_service.h"

#include "base/task/task_traits.h"
#include "base/task/single_thread_task_runner.h"

#include "third_party/boringssl/src/include/openssl/bytestring.h"
#include "third_party/boringssl/src/include/openssl/rsa.h"
#include "third_party/boringssl/src/include/openssl/evp.h"
#include "third_party/boringssl/src/include/openssl/err.h"

#include "base/hash/sha1.h"
#include "base/i18n/time_formatting.h"
#include "base/json/json_reader.h"
#include "base/containers/span.h"
#include "base/base64.h"
#include "crypto/openssl_util.h"
#include "base/json/json_writer.h"
#include "services/network/public/cpp/resource_request.h"
#include "chrome/browser/net/system_network_context_manager.h"
#include "services/network/public/cpp/simple_url_loader.h"
#include "services/network/public/mojom/url_response_head.mojom.h"
#include "net/http/http_status_code.h"
#include "fydeos/prefs/fydeos_pref_names.h"

namespace fydeos {
namespace misc {
namespace {
  constexpr char kJsonContentType[] = "application/json;charset=UTF-8";
  constexpr char kFydeOSLookingGlassPath[] = "/collect";
  constexpr char kShellCmdGetId[] =
    "/usr/share/fydeos_shell/license-utils.sh id";
  constexpr char kShellCmdGetLicense[] =
    "/usr/share/fydeos_shell/license-utils.sh read";
  constexpr char kShellCmdCheckDualBoot[] = "cgpt find -l ";
  constexpr char kDualBootLabel[] = "FYDEOS-DUAL-BOOT";

  const int kStatisticsUploadDelayInSeconds = 7;
  const size_t kMaxMessageSize = 1024;  // 1MB

  fydeos::ash::FydeOSShellClient* GetShellClient() {
    return fydeos::ash::FydeOSShellClient::Get();
  }

  static int LogBoringSSLError(const char* str, size_t len, void* ctx) {
    LOG(ERROR) << std::string_view(str, len);
    return 1;
  }
  static const std::string TimeToString(base::Time time) {
    // base::Time::Exploded ctime;
    // time.UTCExplode(&ctime);
    // const std::string str =
    //    base::StringPrintf("%04d-%02d-%02d %02d:%02d:%02d UTC",
    //    ctime.year, ctime.month, ctime.day_of_month,
    //    ctime.hour, ctime.minute, ctime.second);
    // return str;
    return TimeFormatAsIso8601(time);
  }
}  // namespace

StatisticsCollector::StatisticsCollector() = default;
StatisticsCollector::~StatisticsCollector() = default;

void StatisticsCollector::Start() {
  if (started_) return;
  started_ = true;
  base::SingleThreadTaskRunner::GetCurrentDefault()->PostDelayedTask(
      FROM_HERE,
      base::BindOnce(&StatisticsCollector::StartInternal,
                     weak_factory_.GetWeakPtr()),
      base::Seconds(kStatisticsUploadDelayInSeconds));
}

void StatisticsCollector::Stop() {
  started_ = false;
}

void StatisticsCollector::StartInternal() {
  if (g_browser_process->profile_manager()) {
    profile_ = g_browser_process->profile_manager()->GetActiveUserProfile();
  }
  if (profile_) {
    // TODO(fangzhou@fydeos.io)
    // collect more data if kFydeOSImprovementPlanEnabled is true
    // currently, just make sure the pref kFydeOSImprovementPlanEnabled
    // is saved, it's not being used yet.
    const bool improvementPlanEnabled =
      profile_->GetPrefs()->GetBoolean(
          fydeos::prefs::kFydeOSImprovementPlanEnabled);
    VLOG(3) << "fydeos improvement plan enabled: " << improvementPlanEnabled;
  }
  step_ = CollectStep::INITIALIZE;
  ProceedToNextStep();
}

void StatisticsCollector::ProceedToNextStep() {
  switch (step_) {
    case CollectStep::INITIALIZE:
      GetLicenseId();
      break;
    case CollectStep::GET_LICENSE_ID:
      GetLicenseType();
      break;
    case CollectStep::GET_LICENSE_TYPE:
      GetInstallType();
      break;
    case CollectStep::GET_INSTALL_TYPE:
      SyncCollect();
      break;
    case CollectStep::SYNC_COLLECT:
      UploadStatistics();
      break;
    default:
      return;
  }
}

void StatisticsCollector::GetLicenseId() {
  step_ = CollectStep::GET_LICENSE_ID;
  GetShellClient()->SyncExec(kShellCmdGetId,
      base::BindOnce(&StatisticsCollector::OnGetLicenseId,
                     weak_factory_.GetWeakPtr()));
}

void StatisticsCollector::OnGetLicenseId(
    absl::optional<fydeos::ash::ShellState> state) {
  if (state && state->code == 0) {
    std::string id = state->result;
    id.erase(std::remove(id.begin(), id.end(), '\n'), id.end());
    statistics_.license_id = id;
  }

  ProceedToNextStep();
}

void StatisticsCollector::GetLicenseType() {
  step_ = CollectStep::GET_LICENSE_TYPE;
  GetShellClient()->SyncExec(kShellCmdGetLicense,
      base::BindOnce(&StatisticsCollector::OnGetLicenseType,
                     base::Unretained(this)));
}

void StatisticsCollector::OnGetLicenseType(
    absl::optional<fydeos::ash::ShellState> state) {
  if (state && state->code == 0) {
    GetLicenseTypeFromInfo(state->result);
  }
  ProceedToNextStep();
}

void StatisticsCollector::GetLicenseTypeFromInfo(const std::string& license) {
  absl::optional<base::Value> json = base::JSONReader::Read(license);
  base::Value::Dict* license_dict = nullptr;
  if (!json.has_value()) {
    return;
  }
  license_dict = json->GetIfDict();
  if (!license_dict) {
    return;
  }
  std::string* license_type = license_dict->FindString("license_type");
  if (!license_type) {
    return;
  }
  statistics_.license_type = *license_type;
}

void StatisticsCollector::GetInstallType() {
  step_ = CollectStep::GET_INSTALL_TYPE;
  GetShellClient()->SyncExec(
      std::string(kShellCmdCheckDualBoot) + kDualBootLabel,
      base::BindOnce(
        &StatisticsCollector::OnGetInstallType, base::Unretained(this)));
}

void StatisticsCollector::OnGetInstallType(
    absl::optional<fydeos::ash::ShellState> state) {
  if (!state || state->code != 0) {
    statistics_.is_multi_boot = false;
  } else {
    if (state->result.find("FYDEOS-DUAL-BOOT") != std::string::npos) {
      statistics_.is_multi_boot = true;
    } else {
      statistics_.is_multi_boot = false;
    }
  }

  ProceedToNextStep();
}

void StatisticsCollector::SyncCollect() {
  step_ = CollectStep::SYNC_COLLECT;

  CollectUserInfo();
  CollectDeviceInfo();

  ProceedToNextStep();
}

void StatisticsCollector::CollectUserInfo() {
  if (!profile_) {
    return;
  }
  user_manager::User* user =
    ::ash::ProfileHelper::Get()->GetUserByProfile(profile_);
  if (!user) {
    return;
  }
  const AccountId account_id = user->GetAccountId();
  if (account_id.HasAccountIdKey()) {
    const std::string account_id_hash =
      base::SHA1HashString(account_id.GetAccountIdKey());
    statistics_.profile_account_id = base::Base64Encode(account_id_hash);
  }
  statistics_.profile_account_type =
    AccountId::AccountTypeToString(account_id.GetAccountType());
  statistics_.is_fyde_profile = profile_->IsFydeProfile();
  statistics_.is_new_profile = profile_->IsNewProfile();
  statistics_.profile_start_time = profile_->GetStartTime();
  statistics_.profile_creation_time = profile_->GetCreationTime();
}

void StatisticsCollector::CollectDeviceInfo() {
  statistics_.board_name = base::SysInfo::GetLsbReleaseBoard();
  statistics_.major_version = base::SysInfo::GetLsbFydeReleaseVersion();
  statistics_.kernel_version = base::SysInfo::KernelVersion();
  auto version = chromeos::version_loader::GetVersion(
      chromeos::version_loader::VERSION_SHORT);
  statistics_.os_version = version.value_or("");
  statistics_.os_release_time = base::SysInfo::GetLsbReleaseTime();
  statistics_.hardware_model_name = base::SysInfo::HardwareModelName();
  // const std::string osVersionFull = chromeos::version_loader::GetVersion(
  //    chromeos::version_loader::VERSION_FULL);
  std::string channel_name = chrome::GetChannelName(
                              chrome::WithExtendedStable(false));
  if (channel_name.empty()) {
    channel_name = "stable";
  }
  statistics_.channel_name = channel_name;

  statistics_.browser_language = g_browser_process->GetApplicationLocale();
  statistics_.browser_version = version_info::GetVersionNumber();
  statistics_.browser_milestone = version_info::GetMajorVersionNumber();

  auto provider = ::ash::system::StatisticsProvider::GetInstance();
  if (provider) {
    auto machine_id = provider->GetMachineID();
    if (machine_id) {
      statistics_.sn = machine_id.value();
    }
  }
  // std::string region;
  // provider_->GetMachineStatistic("region", &region);
  // std::string ethernet_mac_address;
  // provider_->GetMachineStatistic(
  //     chromeos::system::kEthernetMacAddressKey, &ethernet_mac_address);
  // std::string dock_mac_address;
  // provider_->GetMachineStatistic(
  //     chromeos::system::kDockMacAddressKey, &dock_mac_address);
}

bool StatisticsCollector::EncryptData(
    const std::string& text, std::string* output) {
  crypto::OpenSSLErrStackTracer err_tracer(FROM_HERE);

  CBS pub_key_cbs;
  CBS_init(&pub_key_cbs,
      fydeos::constants::kFydeOSCryptoKey,
      fydeos::constants::kFydeOSCryptoKeyLength);
  bssl::UniquePtr<EVP_PKEY> pub_key(EVP_parse_public_key(&pub_key_cbs));
  if (!pub_key || CBS_len(&pub_key_cbs)) {
    ERR_print_errors_cb(&LogBoringSSLError, /*unused*/ nullptr);
    return false;
  }

  RSA* rsa = EVP_PKEY_get0_RSA(pub_key.get());
  if (!rsa) {
    ERR_print_errors_cb(&LogBoringSSLError, /*unused*/ nullptr);
    return false;
  }

  // flen must be less than RSA_size(rsa) - 11
  // for the PKCS #1 v1.5 based padding modes,
  // less than RSA_size(rsa) - 41 for RSA_PKCS1_OAEP_PADDING)
  size_t rsa_len;
  const size_t rsa_size = RSA_size(rsa);
  std::vector<uint8_t> result;
  result.reserve(((text.size() / (rsa_size - 42)) + 1) * rsa_size);
  for (unsigned int i = 0; i < text.length(); i += (rsa_size - 42)) {
    const std::string sub = text.substr(i, (rsa_size - 42));
    std::vector<uint8_t> ciphertext(RSA_size(rsa));
    if (!RSA_encrypt(rsa, &rsa_len, ciphertext.data(), ciphertext.size(),
                     reinterpret_cast<const uint8_t *>(sub.data()), sub.size(),
                     RSA_PKCS1_OAEP_PADDING)) {
      ERR_print_errors_cb(&LogBoringSSLError, /*unused*/ nullptr);
      return false;
    }
    result.insert(result.end(), ciphertext.begin(), ciphertext.end());
  }

  *output = base::Base64Encode(std::string(result.begin(), result.end()));
  if (output->empty()) {
    LOG(ERROR) << "Base64Encode error";
    return false;
  }
  return true;
}

bool StatisticsCollector::GetUploadData(std::string* upload_data) {
  base::Value::Dict statistics;
  statistics.Set("license_id", statistics_.license_id);
  statistics.Set("license_type", statistics_.license_type);
  statistics.Set("board", statistics_.board_name);
  statistics.Set("os_version", statistics_.os_version);
  statistics.Set("major_version", statistics_.major_version);
  statistics.Set("sn", statistics_.sn);
  // statistics.Set("kernel", statistics_.kernel_version);
  statistics.Set("is_multi_boot", statistics_.is_multi_boot);
  // statistics.Set("milestone", statistics_.browser_milestone);
  statistics.Set("browser_version", statistics_.browser_version);
  statistics.Set("language", statistics_.browser_language);
  // statistics.Set("channel", statistics_.channel_name);
  statistics.Set("account_id", statistics_.profile_account_id);
  statistics.Set("account_type", statistics_.profile_account_type);
  statistics.Set("is_new_profile", statistics_.is_new_profile);
  // statistics.Set("is_fyde_profile", statistics_.is_fyde_profile);
  statistics.Set("profile_creation",
      TimeToString(statistics_.profile_creation_time));
  statistics.Set("profile_start", TimeToString(statistics_.profile_start_time));
  std::string jsonStr;
  bool write_success = base::JSONWriter::Write(statistics, &jsonStr);
  if (!write_success) {
    LOG(ERROR) << "write json failed, abort";
    return false;
  }

  bool ret = EncryptData(jsonStr, upload_data);
  if (!ret) {
    LOG(ERROR) << "encrypt data failed, abort";
  }

  return ret;
}

bool StatisticsCollector::UploadStatistics() {
  if (!g_browser_process->system_network_context_manager()->HasInstance()) {
    return false;
  }
  network::mojom::URLLoaderFactory* loader_factory =
    g_browser_process->system_network_context_manager()->GetURLLoaderFactory();
  if (!loader_factory) {
    return false;
  }

  std::string payload;
  if (!GetUploadData(&payload)) {
    return false;
  }

  base::Value::Dict post_body_value;
  post_body_value.Set("payload", payload);
  std::string post_body;
  bool write_success = base::JSONWriter::Write(post_body_value, &post_body);
  if (!write_success) {
    return false;
  }
  net::NetworkTrafficAnnotationTag traffic_annotation =
    net::DefineNetworkTrafficAnnotation("fydeos_statistics_upload", R"(
      semantics {
        sender: "FydeOS statistics upload"
        description:
          "upload some anonymous data to fydeos server"
        trigger:
          "after signin"
        data:
          "statistics"
        destination: FYDEOS_STATISTICS_SERVICE
      }
      policy {
        cookies_allowed: NO
        policy_exception_justification: "Not implemented."
      })");

    auto resource_request = std::make_unique<network::ResourceRequest>();
  resource_request->url = GURL(
      fydeos::switches::GetFydeOSLookingGlassUrl() + kFydeOSLookingGlassPath);
  resource_request->credentials_mode = network::mojom::CredentialsMode::kOmit;
  resource_request->method = "POST";
  std::unique_ptr<network::SimpleURLLoader> simple_loader =
    network::SimpleURLLoader::Create(std::move(resource_request),
        traffic_annotation);
  simple_loader->AttachStringForUpload(post_body, kJsonContentType);
  simple_loader->SetTimeoutDuration(base::Minutes(1));
  int retry_mode = network::SimpleURLLoader::RETRY_ON_NETWORK_CHANGE
                   | network::SimpleURLLoader::RETRY_ON_NAME_NOT_RESOLVED;
  simple_loader->SetRetryOptions(3, retry_mode);

  network::SimpleURLLoader* simple_loader_ptr = simple_loader.get();
  simple_loader_ptr->DownloadToString(
      loader_factory,
      base::BindOnce(&StatisticsCollector::OnUploaded,
        weak_factory_.GetWeakPtr(), std::move(simple_loader)),
      kMaxMessageSize);
  return true;
}

void StatisticsCollector::OnUploaded(
    std::unique_ptr<network::SimpleURLLoader> url_loader,
    std::unique_ptr<std::string> response_body) {
  int response_code = 0;
  if (url_loader->ResponseInfo() && url_loader->ResponseInfo()->headers) {
    response_code = url_loader->ResponseInfo()->headers->response_code();
  }
  if (response_code != 200) {
    VLOG(2) << "fydeos service response code: " << response_code;
    if (response_body) {
      VLOG(2) << "received from fydeos service:" << *response_body;
    }
  }
  if (url_loader->NetError() != net::OK) {
    VLOG(2) << "fydeos service net error code:" << url_loader->NetError();
  }
}

}  // namespace misc
}  // namespace fydeos
