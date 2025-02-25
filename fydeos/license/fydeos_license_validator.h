// Copyright 2020 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FYDEOS_LICENSE_VALIDATOR_H_
#define FYDEOS_LICENSE_VALIDATOR_H_
#include <stdint.h>

#include <memory>
#include <string>
#include <vector>
#include "base/values.h"
#include "fydeos/license/fydeos_callback_status.h"
#include "base/json/json_reader.h"
#include "crypto/signature_verifier.h"

namespace fydeos {
namespace license {
class LicenseValidator {
 public:
  LicenseValidator();
  ~LicenseValidator();
  void StartValidate(const std::string& id, std::optional<std::string> license,
    SuccessCallback<std::optional<base::Value>> success_callback,
    SavePrefCallback save_pref_callback,
    ErrorWithSaveCallback<std::optional<base::Value>> err_callback);

 private:
  crypto::SignatureVerifier signature_verifier_;
};

}  // namespace license
}  // namespace fydeos


#endif  // FYDEOS_LICENSE_VALIDATOR_H_
