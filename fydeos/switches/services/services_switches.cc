// Copyright (c) 2019 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fydeos/switches/services/services_switches.h"

#include "base/command_line.h"
#include "fydeos/switches/services/services_constants.h"
#include "fydeos/switches/urls/urls_constants.h"
#include "fydeos/build/config/buildflags.h"

namespace fydeos {
namespace switches {

namespace {

const char kDisableFydeOSGeolocationAPI[] = "disable-fydeos-geolocation-api";
const char kDisableFydeOSTimezoneAPI[] = "disable-fydeos-timezone-api";

const char kFydeOSGeolocationAPIUrl[] = "fydeos-geolocation-api-url";
const char kFydeOSTimezoneAPIUrl[] = "fydeos-timezone-api-url";

const char kFydeOSLookingGlassUrl[] = "fydeos-lookingglass-url";

const char kFydeOSAppsGalleryURL[] = "fydeos-apps-gallery-url";

const char kFydeOSAppsGalleryUpdateURL[] = "fydeos-apps-gallery-update-url";

const char kFydeOSAssistantWebUrl[] = "fydeos-ai-url";

const char kFydeOSStoreComPrefix[] = "https://store.fydeos.com";
const char kFydeOSStoreIoPrefix[] = "https://store.fydeos.io";

#if BUILDFLAG(FYDEOS_DEVICE)
const char kFydeOSProductWarrantyURL[] = "fydeos-product-warranty-url";
#endif

}

bool DisableFydeOSGeolocationAPI() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  return command_line->HasSwitch(kDisableFydeOSGeolocationAPI);
}

bool DisableFydeOSTimezoneAPI() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  return command_line->HasSwitch(kDisableFydeOSTimezoneAPI);
}

std::string GetFydeOSGeolocationAPIUrl() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(kFydeOSGeolocationAPIUrl)) {
    return command_line->GetSwitchValueASCII(kFydeOSGeolocationAPIUrl);
  } else {
    return std::string(fydeos::constants::kDefaultFydeOSGeolocationAPIUrl);
  }
}

std::string GetFydeOSTimezoneAPIUrl() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(kFydeOSTimezoneAPIUrl)) {
    return command_line->GetSwitchValueASCII(kFydeOSTimezoneAPIUrl);
  } else {
    return std::string(fydeos::constants::kDefaultFydeOSTimezoneAPIUrl);
  }
}

std::string GetFydeOSLookingGlassUrl() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(kFydeOSLookingGlassUrl)) {
    return command_line->GetSwitchValueASCII(kFydeOSLookingGlassUrl);
  } else {
    return std::string(fydeos::constants::kDefaultFydeOSLookingGlassUrl);
  }
}

std::string GetFydeOSAppStoreURL() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(kFydeOSAppsGalleryURL)) {
    return command_line->GetSwitchValueASCII(kFydeOSAppsGalleryURL);
  } else {
    return std::string(fydeos::constants::kFydeOSStoreBaseUrl);
  }
}

std::string GetFydeOSWebStoreUpdateUrl() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(kFydeOSAppsGalleryUpdateURL)) {
    return command_line->GetSwitchValueASCII(kFydeOSAppsGalleryUpdateURL);
  } else {
    return std::string(fydeos::constants::kFydeOSWebStoreUpdateURL);
  }
}

std::string GetFydeOSAssistantWebUrl() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(kFydeOSAssistantWebUrl)) {
    std::string value(command_line->GetSwitchValueASCII(kFydeOSAssistantWebUrl));
    if (!value.empty()) {
      return value;
    }
  }
  return std::string(fydeos::constants::kFydeOSAssistantDefaultWebUrl);
}

CHROMEOS_EXPORT std::string MayConvertWebStoreUpdateUrl(
    const std::string& url) {
  std::string new_url = url;
#if BUILDFLAG(USE_FYDEOS_COM)
  if (new_url.find(kFydeOSStoreIoPrefix) == 0) {
    new_url.replace(0, sizeof(kFydeOSStoreIoPrefix) - 1,
                    kFydeOSStoreComPrefix);
  }
#else
  if (new_url.find(kFydeOSStoreComPrefix) == 0) {
    new_url.replace(0, sizeof(kFydeOSStoreComPrefix) - 1,
                    kFydeOSStoreIoPrefix);
  }
#endif
  return new_url;
}

#if BUILDFLAG(FYDEOS_DEVICE)
std::string GetFydeOSProductWarrantyUrl() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(kFydeOSProductWarrantyURL)) {
    return command_line->GetSwitchValueASCII(kFydeOSProductWarrantyURL);
  } else {
    return std::string(fydeos::constants::kFydeOSProductWarrantyDefaultURL);
  }
}
#endif

} // switches
} // fydeos
