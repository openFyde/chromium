// Copyright (c) 2019 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
#ifndef CHROMEOS_FYDEOS_SWITCHES_ACCOUNT_POLICY_CONSTANTS_H_
#define CHROMEOS_FYDEOS_SWITCHES_ACCOUNT_POLICY_CONSTANTS_H_

#include <string>
#include "chromeos/chromeos_export.h"

namespace fydeos {
namespace constants {

CHROMEOS_EXPORT extern const uint8_t kFydeOSPolicyVerificationKey[];
extern const size_t kFydeOSPolicyVerificationKeyLength;

CHROMEOS_EXPORT extern const char kFydeOSPolicyFCMInvalidationSenderID[];

}
}

#endif
