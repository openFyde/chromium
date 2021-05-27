// Copyright (c) 2019 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_FYDEOS_SWITCHES_DISPLAY_SWITCHES_H_
#define CHROMEOS_FYDEOS_SWITCHES_DISPLAY_SWITCHES_H_

#include "chromeos/chromeos_export.h"

namespace fydeos {
namespace switches {

CHROMEOS_EXPORT float GetDefaultDSF(float default_value);
CHROMEOS_EXPORT float GetDefaultScreenDpi(float default_value);

} // switches
} // fydeos

#endif /* ifndef CHROMEOS_FYDEOS_SWITCHES_DISPLAY_SWITCHES_H_ */
