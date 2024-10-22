// Copyright 2024 The Fyde Innovations Limited. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fydeos/constants/fydeos_constants.h"

namespace fydeos::constants {

namespace {

const char kFydeSystemControllerExtensionId[] = "mofiofjpikncjaigmdlblhojbnkabako";
const char kFydeRdpExtensionId[] = "fogdcaodknbhigpklbhepedofamkfbln";

constexpr char const* kExtensionIdsHiddenInList[] = {
    kFydeSystemControllerExtensionId,
    kFydeRdpExtensionId,
};

}

bool ShouldHideExtensionById(const std::string& extension_id) {
  for (auto* const id : kExtensionIdsHiddenInList) {
    if (id == extension_id) {
      return true;
    }
  }
  return false;
}

} // namespace fydeos::constants
