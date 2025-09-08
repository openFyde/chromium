// Copyright 2022 Fyde Innovations. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fydeos/chrome/browser/web_applications/preinstalled_web_apps/community.h"
#include "fydeos/switches/urls/urls_constants.h"

namespace web_app {

ExternalInstallOptions GetConfigForFydeNotes() {
  ExternalInstallOptions options(
      /*install_url=*/GURL(fydeos::constants::kFydeOSNotesAppURL),
      /*user_display_mode=*/mojom::UserDisplayMode::kStandalone,
      /*install_source=*/ExternalInstallSource::kExternalDefault);

  // Exclude managed users until we have a way for admins to block the app.
  options.user_type_allowlist = {"unmanaged", "managed", "child"};
  options.add_to_quick_launch_bar = false;

  return options;
}

}  // namespace web_app
