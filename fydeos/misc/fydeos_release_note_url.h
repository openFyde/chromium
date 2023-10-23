// Copyright (c) 2021 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _FYDEOS_MISC_FYDEOS_RELEASE_NOTE_URL_H_
#define _FYDEOS_MISC_FYDEOS_RELEASE_NOTE_URL_H_

#include <string>

class Profile;

namespace fydeos {
namespace misc {

const std::string BuildFydeReleaseNoteUrlWithPath(Profile* profile);

}
}

#endif  // _FYDEOS_MISC_FYDEOS_RELEASE_NOTE_URL_H_
