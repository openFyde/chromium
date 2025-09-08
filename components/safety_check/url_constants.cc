// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/safety_check/url_constants.h"
#include "fydeos/build/config/buildflags.h"

namespace safety_check {

#if BUILDFLAG(USE_FYDEOS_COM)
const char kConnectivityCheckUrl[] =
    "https://store.fydeos.com/204";
#else
const char kConnectivityCheckUrl[] =
    "https://connectivitycheck.gstatic.com/generate_204";
#endif

}  // namespace safety_check
