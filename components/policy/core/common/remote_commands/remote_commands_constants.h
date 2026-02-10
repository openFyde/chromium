// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_POLICY_CORE_COMMON_REMOTE_COMMANDS_REMOTE_COMMANDS_CONSTANTS_H_
#define COMPONENTS_POLICY_CORE_COMMON_REMOTE_COMMANDS_REMOTE_COMMANDS_CONSTANTS_H_

#include <stdint.h>

#include "components/invalidation/invalidation_constants.h"
#include "fydeos/switches/account/policy_constants.h"
#include "fydeos/switches/misc/misc_switches.h"

namespace policy {

// GCP number to be used for remote commands invalidations. Remote commands
// are considered critical to receive invalidation.
inline constexpr int64_t kRemoteCommandsInvalidationsProjectNumber =
    invalidation::kCriticalInvalidationsProjectNumber;

inline int64_t GetRemoteCommandsInvalidationsProjectNumber() {
  if (fydeos::switches::UseFydeInvalidationService()) {
    return fydeos::constants::kFydeOSPolicyFCMInvalidationSenderID;
  }
  return kRemoteCommandsInvalidationsProjectNumber;
}

}  // namespace policy

#endif  // COMPONENTS_POLICY_CORE_COMMON_REMOTE_COMMANDS_REMOTE_COMMANDS_CONSTANTS_H_
