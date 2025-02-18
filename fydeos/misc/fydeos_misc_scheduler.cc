// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fydeos/misc/fydeos_misc_scheduler.h"
#include "fydeos/misc/fydeos_statistics_collector.h"
#include <base/logging.h>

namespace fydeos {
namespace misc {

namespace {
  FydeMiscScheduler* g_misc_scheduler = nullptr;
}

void FydeMiscScheduler::Initialize() {
  CHECK(g_misc_scheduler == nullptr);
  g_misc_scheduler = new FydeMiscScheduler();
}

FydeMiscScheduler* FydeMiscScheduler::Get() {
  return g_misc_scheduler;
}

void FydeMiscScheduler::Shutdown() {
  CHECK(g_misc_scheduler != nullptr);
  g_misc_scheduler->Stop();
  delete g_misc_scheduler;
  g_misc_scheduler = nullptr;
}

FydeMiscScheduler::FydeMiscScheduler() :
  collector_(std::make_unique<StatisticsCollector>()) {
  if (::ash::LoginState::IsInitialized()) {
    ::ash::LoginState::Get()->AddObserver(this);
  }
}

FydeMiscScheduler::~FydeMiscScheduler() = default;

void FydeMiscScheduler::Start() {
  if (started_) return;
  VLOG(2) << "FydeMiscScheduler Start";
  started_ = true;
  collector_->Start();
}

void FydeMiscScheduler::Stop() {
  VLOG(2) << "FydeMiscScheduler Stop";
  collector_->Stop();
  started_ = false;
}

void FydeMiscScheduler::LoggedInStateChanged() {
  if (::ash::LoginState::Get()->IsUserLoggedIn() && ::ash::LoginState::Get()->IsUserAuthenticated()) {
    Start();
  } else {
    Stop();
  }
}

}  // namespace misc
}  // namespace fydeos
