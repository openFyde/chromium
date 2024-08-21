// Copyright 2023 The Fyde Innovations Limited. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef FYDEOS_EXTENSIONS_BROWSER_API_FYDEOS_PRIVATE_FYDEOS_PRIVATE_API_H_
#define FYDEOS_EXTENSIONS_BROWSER_API_FYDEOS_PRIVATE_FYDEOS_PRIVATE_API_H_

#include "extensions/browser/extension_function.h"
#include "extensions/browser/extension_function_histogram_value.h"

namespace extensions {
class FydeosPrivateGetFydeOSInfoFunction: public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("fydeosPrivate.getFydeOSInfo",
                              FYDEOS_PRIVATE_GET_FYDEOS_INFO)

 protected:
  ~FydeosPrivateGetFydeOSInfoFunction() override = default;
  ResponseAction Run() override;
};
}  // namespace extensions


#endif  // FYDEOS_EXTENSIONS_BROWSER_API_FYDEOS_PRIVATE_FYDEOS_PRIVATE_API_H_
