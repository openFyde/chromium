// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/webui/diagnostics_ui/backend/session_log_handler.h"

#include "ash/constants/ash_features.h"
#include "ash/public/cpp/holding_space/holding_space_client.h"
#include "ash/system/diagnostics/diagnostics_log_controller.h"
#include "ash/system/diagnostics/networking_log.h"
#include "ash/system/diagnostics/routine_log.h"
#include "ash/system/diagnostics/telemetry_log.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/files/file.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/task/thread_pool.h"
#include "base/values.h"
#include "chromeos/strings/grit/chromeos_strings.h"
#include "content/public/browser/web_contents.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/shell_dialogs/select_file_policy.h"

#include "chrome/browser/feedback/system_logs/about_system_logs_fetcher.h"
#include "components/feedback/system_logs/system_logs_fetcher.h"
#include "third_party/zlib/google/zip.h"

#include "fydeos/switches/misc/misc_constants.h"

using system_logs::SystemLogsResponse;

namespace ash {
namespace diagnostics {
namespace {

const char kRoutineLogSubsectionHeader[] = "--- Test Routines --- \n";
const char kSystemLogSectionHeader[] = "=== System === \n";
const char kNetworkingLogSectionHeader[] = "=== Networking === \n";
const char kNoRoutinesRun[] =
    "No routines of this type were run in the session.\n";
const char kDefaultSessionLogFileName[] = "about_system.zip";

std::string GetRoutineResultsString(const std::string& results) {
  const std::string section_header =
      std::string(kRoutineLogSubsectionHeader) + "\n";
  if (results.empty()) {
    return section_header + kNoRoutinesRun;
  }

  return section_header + results;
}

}  // namespace

SessionLogHandler::SessionLogHandler(
    const SelectFilePolicyCreator& select_file_policy_creator,
    ash::HoldingSpaceClient* holding_space_client,
    const base::FilePath& log_directory_path)
    : SessionLogHandler(select_file_policy_creator,
                        std::make_unique<TelemetryLog>(),
                        std::make_unique<RoutineLog>(log_directory_path),
                        std::make_unique<NetworkingLog>(log_directory_path),
                        holding_space_client) {}

SessionLogHandler::SessionLogHandler(
    const SelectFilePolicyCreator& select_file_policy_creator,
    std::unique_ptr<TelemetryLog> telemetry_log,
    std::unique_ptr<RoutineLog> routine_log,
    std::unique_ptr<NetworkingLog> networking_log,
    ash::HoldingSpaceClient* holding_space_client)
    : select_file_policy_creator_(select_file_policy_creator),
      telemetry_log_(std::move(telemetry_log)),
      routine_log_(std::move(routine_log)),
      networking_log_(std::move(networking_log)),
      holding_space_client_(holding_space_client),
      task_runner_(
          base::ThreadPool::CreateSequencedTaskRunner({base::MayBlock()})) {
  DCHECK(holding_space_client_);
  weak_ptr_ = weak_factory_.GetWeakPtr();
}

SessionLogHandler::~SessionLogHandler() {
  if (select_file_dialog_) {
    /* Lifecycle for SelectFileDialog is responsibility of calling code. */
    select_file_dialog_->ListenerDestroyed();
  }
}

void SessionLogHandler::RegisterMessages() {
  web_ui()->RegisterDeprecatedMessageCallback(
      "initialize",
      base::BindRepeating(&SessionLogHandler::HandleInitialize, weak_ptr_));
  web_ui()->RegisterDeprecatedMessageCallback(
      "saveSessionLog",
      base::BindRepeating(&SessionLogHandler::HandleSaveSessionLogRequest,
                          weak_ptr_));
  web_ui()->RegisterDeprecatedMessageCallback(
      "getFydeOsSystemInfo",
      base::BindRepeating(&SessionLogHandler::HandleGetFydeOsSystemInfo,
                          weak_ptr_));
}

void SessionLogHandler::FileSelected(const base::FilePath& path,
                                     int index,
                                     void* params) {
  // TODO(b/226574520): Remove SessionLogHandler::CreateSessionLog and
  // condition as part of flag clean up.
  if (ash::features::IsLogControllerForDiagnosticsAppEnabled()) {
    task_runner_->PostTaskAndReplyWithResult(
        FROM_HERE,
        base::BindOnce(
            &DiagnosticsLogController::GenerateSessionLogOnBlockingPool,
            base::Unretained(DiagnosticsLogController::Get()), path),
        base::BindOnce(&SessionLogHandler::OnSessionLogCreated, weak_ptr_,
                       path));
  } else {
    bool ret;

    ret = PrepareSessionLog(path);
    if (!ret) {
      LOG(ERROR) << "failed to prepare "<< path;
      return;
    }

    ret = CreateSessionLog(fydeos_system_info_temp_path_);
    if (!ret) {
      LOG(ERROR) << "failed to create " << fydeos_system_info_temp_path_;
      return;
    }

    task_runner_->PostTaskAndReplyWithResult(
        FROM_HERE,
        base::BindOnce(&SessionLogHandler::CompressSessionLog,
                       base::Unretained(this), fydeos_system_info_temp_path_, path),
        base::BindOnce(&SessionLogHandler::OnSessionLogCreated, weak_ptr_,
                       path));
  }
  select_file_dialog_.reset();
}

void SessionLogHandler::OnSessionLogCreated(const base::FilePath& file_path,
                                            bool success) {
  if (success) {
    holding_space_client_->AddDiagnosticsLog(file_path);
  }

  ResolveJavascriptCallback(base::Value(save_session_log_callback_id_),
                            base::Value(success));
  save_session_log_callback_id_ = "";

  if (log_created_closure_)
    std::move(log_created_closure_).Run();
}

void SessionLogHandler::FileSelectionCanceled(void* params) {
  RejectJavascriptCallback(base::Value(save_session_log_callback_id_),
                           /*success=*/base::Value(false));
  save_session_log_callback_id_ = "";
  select_file_dialog_.reset();
}

TelemetryLog* SessionLogHandler::GetTelemetryLog() const {
  return telemetry_log_.get();
}

RoutineLog* SessionLogHandler::GetRoutineLog() const {
  return routine_log_.get();
}

NetworkingLog* SessionLogHandler::GetNetworkingLog() const {
  return networking_log_.get();
}

void SessionLogHandler::SetTaskRunnerForTesting(
    const scoped_refptr<base::SequencedTaskRunner>& task_runner) {
  task_runner_ = std::move(task_runner.get());
}

void SessionLogHandler::SetWebUIForTest(content::WebUI* web_ui) {
  set_web_ui(web_ui);
}

void SessionLogHandler::SetLogCreatedClosureForTest(base::OnceClosure closure) {
  log_created_closure_ = std::move(closure);
}

bool SessionLogHandler::CreateSessionLog(const base::FilePath& file_path) {
  // Fetch Routine logs
  const std::string system_routines =
      routine_log_->GetContentsForCategory("system");
  const std::string network_routines =
      routine_log_->GetContentsForCategory("network");

  // Fetch system data from TelemetryLog.
  const std::string system_log_contents = telemetry_log_->GetContents();

  std::vector<std::string> pieces;
  pieces.push_back(kSystemLogSectionHeader);

  if (!fydeos_system_info_.empty()) {
    pieces.push_back(fydeos::constants::kFydeOSSystemInfoHeader);
    pieces.push_back(fydeos_system_info_);
  }

  if (!system_log_contents.empty()) {
    pieces.push_back(system_log_contents);
  }

  // Add the routine section for the system category.
  pieces.push_back(GetRoutineResultsString(system_routines));

  if (features::IsNetworkingInDiagnosticsAppEnabled()) {
    // Add networking category.
    pieces.push_back(kNetworkingLogSectionHeader);

    // Add the network info section.
    pieces.push_back(networking_log_->GetNetworkInfo());

    // Add the routine section for the network category.
    pieces.push_back(GetRoutineResultsString(network_routines));

    // Add the network events section.
    pieces.push_back(networking_log_->GetNetworkEvents());
  }

  return base::WriteFile(file_path, base::JoinString(pieces, "\n"));
}

void SessionLogHandler::HandleGetFydeOsSystemInfo(const base::ListValue* args) {
  system_logs::SystemLogsFetcher* fetcher =
    system_logs::BuildAboutSystemLogsFetcher();
  fetcher->Fetch(base::BindOnce(&SessionLogHandler::OnFydeOSSystemInfoReceived,
                                weak_factory_.GetWeakPtr()));
}

void SessionLogHandler::OnFydeOSSystemInfoReceived(std::unique_ptr<SystemLogsResponse> sys_info) {
  if (!sys_info) {
    LOG(WARNING) << "Failed to get FydeOS system info";
    return;
  }

  fydeos_system_info_ = "";

  for (SystemLogsResponse::const_iterator it = sys_info->begin();
       it != sys_info->end(); ++it) {
    auto val = std::make_unique<base::DictionaryValue>();
    fydeos_system_info_ += it->first;
    fydeos_system_info_ += ":\n";
    fydeos_system_info_ += it->second;
    fydeos_system_info_ += "\n";
  }
}

bool SessionLogHandler::FydeosCreateSystemInfoTempDirectory() {
  if (!base::CreateNewTempDirectory(FILE_PATH_LITERAL(fydeos::constants::kFydeOSSystemTempPrefix),
                                    &fydeos_system_info_temp_path_))
    return false;

  fydeos_system_info_temp_path_ = fydeos_system_info_temp_path_.Append(base::FilePath(fydeos::constants::kFydeOSSystemInfoFileName));

  return true;
}

bool SessionLogHandler::CompressSessionLog(const base::FilePath& file_path,
                                           const base::FilePath& dest) {

  const base::FilePath base_dir = file_path.DirName();

  base::File file(dest, base::File::FLAG_CREATE_ALWAYS | base::File::FLAG_WRITE);
  if (!file.IsValid())
    return false;

  std::vector<base::FilePath> files_to_zip;
  files_to_zip.push_back(file_path.BaseName());
  return zip::ZipFiles(base_dir, files_to_zip, file.GetPlatformFile());
}

bool SessionLogHandler::PrepareSessionLog(const base::FilePath& file_path) {
  FydeosCreateSystemInfoTempDirectory();
  return true;
}

void SessionLogHandler::HandleSaveSessionLogRequest(
    const base::ListValue* args) {
  CHECK_EQ(1U, args->GetListDeprecated().size());
  DCHECK(save_session_log_callback_id_.empty());
  save_session_log_callback_id_ = args->GetListDeprecated()[0].GetString();

  content::WebContents* web_contents = web_ui()->GetWebContents();
  gfx::NativeWindow owning_window =
      web_contents ? web_contents->GetTopLevelNativeWindow()
                   : gfx::kNullNativeWindow;

  // Early return if the select file dialog is already active.
  if (select_file_dialog_)
    return;
  select_file_dialog_ = ui::SelectFileDialog::Create(
      this, select_file_policy_creator_.Run(web_contents));
  select_file_dialog_->SelectFile(
      ui::SelectFileDialog::SELECT_SAVEAS_FILE,
      /*title=*/l10n_util::GetStringUTF16(IDS_DIAGNOSTICS_SELECT_DIALOG_TITLE),
      /*default_path=*/base::FilePath(kDefaultSessionLogFileName),
      /*file_types=*/nullptr,
      /*file_type_index=*/0,
      /*default_extension=*/base::FilePath::StringType(), owning_window,
      /*params=*/nullptr);
}

void SessionLogHandler::HandleInitialize(const base::ListValue* args) {
  DCHECK(args && args->GetListDeprecated().empty());
  AllowJavascript();
}

}  // namespace diagnostics
}  // namespace ash
