// Copyright 2020 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FYDEOS_LICENSE_USER_UTIL_
#define FYDEOS_LICENSE_USER_UTIL_

class GURL;

namespace fydeos {
namespace license {

GURL AppendAccountIdQueryParameter(GURL& url);

} // license
} // fydeos

#endif
