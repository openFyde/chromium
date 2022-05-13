// Copyright (c) 2019 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_FYDEOS_SWITCHES_DISPLAY_SWITCHES_H_
#define CHROMEOS_FYDEOS_SWITCHES_DISPLAY_SWITCHES_H_

#include "chromeos/chromeos_export.h"
#define EXPORT_ALL __attribute__((visibility("default")))

namespace fydeos {
namespace switches {
EXPORT_ALL extern const char kForceLegacyPlaneManager[];
EXPORT_ALL extern const char kForceShowCursor[];
EXPORT_ALL extern const char kUseRpiVideoDecoder[];
EXPORT_ALL bool ForceLegacyPlaneManager();
EXPORT_ALL bool ForceShowCursor();
CHROMEOS_EXPORT bool UseRpiVideoDecoder();

CHROMEOS_EXPORT float GetDefaultDSF(float default_value);
CHROMEOS_EXPORT float GetDefaultScreenDpi(float default_value);

CHROMEOS_EXPORT bool ForceCursorCompositing();

} // switches
} // fydeos

#endif /* ifndef CHROMEOS_FYDEOS_SWITCHES_DISPLAY_SWITCHES_H_ */
