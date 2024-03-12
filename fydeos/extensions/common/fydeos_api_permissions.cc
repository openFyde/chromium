// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fydeos/extensions/common/fydeos_api_permissions.h"

#include <stddef.h>

#include <memory>

#include "extensions/common/permissions/api_permission.h"

namespace extensions {

namespace {

	template <typename T>
	std::unique_ptr<APIPermission> CreateAPIPermission(
		const APIPermissionInfo* permission) {
	  return std::make_unique<T>(permission);
	}

  constexpr APIPermissionInfo::InitInfo permissions_to_register[] = {
      // Register permissions for all extension types.
    {mojom::APIPermissionID::kFydeOSPrivate, "FydeOSPrivate",
       APIPermissionInfo::kFlagCannotBeOptional},
    {mojom::APIPermissionID::kFydeOSShellClient, "FydeOSShellClient"}
  };

}  // namespace

namespace fydeos_api_permissions {
	base::span<const APIPermissionInfo::InitInfo> GetPermissionInfos() {
	  return base::make_span(permissions_to_register);
	}

	base::span<const Alias> GetPermissionAliases() {
	  // In alias constructor, first value is the alias name; second value is the
	  // real name. See also alias.h.
	  return base::span<const extensions::Alias>();
	}
} //fydeos_api_permissions
}  // namespace extensions
