// Copyright 2023 The Fyde Innovations Limited. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fydeos/extensions/browser/api/fydeos_private/fydeos_private_api.h"
#include "fydeos/build/config/buildflags.h"
#include "base/command_line.h"
#include "fydeos/switches/misc/misc_switches.h"

namespace extensions {

ExtensionFunction::ResponseAction FydeosPrivateGetFydeOSInfoFunction::Run() {
  base::Value::Dict result;
#if BUILDFLAG(USE_FYDEOS_COM)
  std::string host_sufffux = "fydeos.com";
#else
  std::string host_sufffux = "fydeos.io";
#endif
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(
        fydeos::switches::kFydeOSServiceHostSuffixForTesting)) {
    host_sufffux = command_line->GetSwitchValueASCII(
        fydeos::switches::kFydeOSServiceHostSuffixForTesting);
  }
  result.Set("host_suffix", host_sufffux);
#if BUILDFLAG(IS_OPENFYDE)
  result.Set("fydeos", false);
#else
  result.Set("fydeos", true);
#endif
  return RespondNow(WithArguments(std::move(result)));
}

}  // namespace extensions
