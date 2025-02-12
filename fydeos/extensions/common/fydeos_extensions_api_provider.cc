// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fydeos/extensions/common/fydeos_extensions_api_provider.h"

#include "fydeos/extensions/common/api/api_features.h"
#include "fydeos/extensions/common/api/generated_schemas.h"
#include "fydeos/extensions/common/api/permission_features.h"
#include "fydeos/extensions/common/fydeos_api_permissions.h"
#include "fydeos/extensions/common/grit/fydeos_extensions_resources.h"
#include "extensions/common/features/json_feature_provider_source.h"
#include "extensions/common/permissions/permissions_info.h"
#include "base/logging.h"

namespace extensions {

FydeOSExtensionsAPIProvider::FydeOSExtensionsAPIProvider() {}
FydeOSExtensionsAPIProvider::~FydeOSExtensionsAPIProvider() = default;

void FydeOSExtensionsAPIProvider::AddAPIFeatures(FeatureProvider* provider) {
  AddFydeOSAPIFeatures(provider);
  VLOG(1) << "Add fydeos api features\n";
}

void FydeOSExtensionsAPIProvider::AddManifestFeatures(
    FeatureProvider* provider) {
}

void FydeOSExtensionsAPIProvider::AddPermissionFeatures(
    FeatureProvider* provider) {
  AddFydeOSPermissionFeatures(provider);
  VLOG(1) << "Add fydeos permission features\n";
}

void FydeOSExtensionsAPIProvider::AddBehaviorFeatures(
    FeatureProvider* provider) {
  // Note: No chrome-specific behavior features.
}

void FydeOSExtensionsAPIProvider::AddAPIJSONSources(
    JSONFeatureProviderSource* json_source) {
  json_source->LoadJSON(IDR_FYDEOS_EXTENSION_API_FEATURES);
}

bool FydeOSExtensionsAPIProvider::IsAPISchemaGenerated(
    const std::string& name) {
  return api::FydeOSGeneratedSchemas::IsGenerated(name);
}

base::StringPiece FydeOSExtensionsAPIProvider::GetAPISchema(
    const std::string& name) {
  return api::FydeOSGeneratedSchemas::Get(name);
}

void FydeOSExtensionsAPIProvider::RegisterPermissions(PermissionsInfo* permissions_info) {
  permissions_info->RegisterPermissions(
      fydeos_api_permissions::GetPermissionInfos(),
      fydeos_api_permissions::GetPermissionAliases());
  VLOG(1) << "register fydeos permissions";
}

void FydeOSExtensionsAPIProvider::RegisterManifestHandlers() {
}

}  // namespace extensions
