// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FYDEOS_MISC_SCHEDULER_H_
#define FYDEOS_MISC_SCHEDULER_H_

#include <memory>
#include "chromeos/chromeos_export.h"
#include "chromeos/ash/components/login/login_state/login_state.h"
#include "fydeos/build/config/buildflags.h"

namespace fydeos {
namespace misc {

class StatisticsCollector;
class MiscCrostiniNotifier;

class CHROMEOS_EXPORT FydeMiscScheduler : public ::ash::LoginState::Observer {
 public:
    FydeMiscScheduler();
    ~FydeMiscScheduler() override;
    static void Initialize();
    static FydeMiscScheduler* Get();
    static void Shutdown();
    void Start();
    void Stop();

 private:
    void LoggedInStateChanged() override;

    bool started_ = false;
#if BUILDFLAG(USE_FYDEOS_COM)
    std::unique_ptr<MiscCrostiniNotifier> crostini_notifier_;
#endif
    std::unique_ptr<StatisticsCollector> collector_;
};

}  // namespace misc
}  // namespace fydeos

#endif /* ifndef FYDEOS_MISC_SCHEDULER_H_ */
