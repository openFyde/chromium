// Copyright 2023 Fyde Innovations. All rights reserved.

#include "fydeos/extensions/browser/api/fydeos_arc_status/fydeos_arc_status_api.h"
#include "base/files/file_util.h"

namespace extensions {

namespace {

const char kArcImageBasePath[] = "/opt/google/containers/android";
const char kArcSystemImageName[] = "system.raw.img";
const char kArcVendorImageName[] = "vendor.raw.img";

const char kArcRunningProcessName[] = "org.chromium.arc.home";
const char kArcInstallableProcessName[] = "/system/bin/arc_fydeos_service";

const char kArcSupportedKey[] = "supported";
const char kArcHeroKey[] = "archero";
const char kArcRunningKey[] = "running";
const char kArcInstallableKey[] = "installable";

const char kArcHeroProcessSuffix[] = "com.fydeos.archero.";

bool GetArcSupported() {
  bool system_img_exists = base::PathExists(
      base::FilePath(kArcImageBasePath).Append(kArcSystemImageName));
  bool vendor_img_exists = base::PathExists(
      base::FilePath(kArcImageBasePath).Append(kArcVendorImageName));
  return system_img_exists && vendor_img_exists;
}

void ArcStatusHelper::GetStatus(StatusCallback callback) {
  callback_ = std::move(callback);

  status_.archero = false;
  status_.installable = false;
  status_.running = false;
  status_.supported = GetArcSupported();

  GetSystemProcessList();
}

void ArcStatusHelper::GetSystemProcessList() {
  arc::ArcProcessService* arc_process_service = arc::ArcProcessService::Get();
  if (!arc_process_service) {
    Final();
    return;
  }
  arc_process_service->RequestSystemProcessList(
      base::BindOnce(&ArcStatusHelper::OnGetSystemProcessList, weak_factory_.GetWeakPtr()));
}

void ArcStatusHelper::OnGetSystemProcessList(
    OptionalArcProcessList processes) {
  if (!processes) {
    Final();
    return;
  }
  for (auto& entry : *processes) {
    if (entry.process_name() == kArcRunningProcessName) {
      status_.running = true;
    } else if (entry.process_name() == kArcInstallableProcessName) {
      status_.installable = true;
    } else if (base::StartsWith(entry.process_name(), kArcHeroProcessSuffix)) {
      // it's a special case
      status_.archero = true;
      status_.running = true;
      status_.supported = true;
      status_.installable = true;
      break;
    }
    if (status_.installable && status_.running) {
      break;
    }
  }

  GetAppProcessList();
}

void ArcStatusHelper::GetAppProcessList() {
  arc::ArcProcessService* arc_process_service = arc::ArcProcessService::Get();
  if (!arc_process_service) {
    Final();
    return;
  }
  arc_process_service->RequestAppProcessList(
      base::BindOnce(&ArcStatusHelper::OnGetAppProcessList, weak_factory_.GetWeakPtr()));
}

void ArcStatusHelper::OnGetAppProcessList(
    OptionalArcProcessList processes) {
  if (!processes) {
    Final();
    return;
  }
  for (auto& entry : *processes) {
    // check only kArcRunningProcessName here
    if (entry.process_name() == kArcRunningProcessName) {
      status_.running = true;
      break;
    }
  }
  Final();
}

void ArcStatusHelper::Final() {
  if (callback_) {
    std::move(callback_).Run(status_);
  }
}

ArcStatusHelper *helper = nullptr;

}  // namespace

ExtensionFunction::ResponseAction FydeosArcStatusGetFunction::Run() {
  if (!helper) {
    helper = new ArcStatusHelper();
  }
  helper->GetStatus(base::BindOnce(
      &FydeosArcStatusGetFunction::OnGetStatus, this));

  return RespondLater();
}

void FydeosArcStatusGetFunction::OnGetStatus(ArcStatus status) {
  base::Value result(base::Value::Type::DICT);
  result.SetBoolKey(kArcSupportedKey, status.supported);
  result.SetBoolKey(kArcHeroKey, status.archero);
  result.SetBoolKey(kArcRunningKey, status.running);
  result.SetBoolKey(kArcInstallableKey, status.installable);
  Respond(WithArguments(std::move(result)));
}

}
