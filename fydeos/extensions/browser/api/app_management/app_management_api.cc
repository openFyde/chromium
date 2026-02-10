// Copyright (c) 2018 The FlintOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// Author: Simon Tsao(yang@flintos.io)

#include "fydeos/extensions/browser/api/app_management/app_management_api.h"

#include <stddef.h>
#include <algorithm>
#include <limits>
#include <utility>
#include <vector>

#include "base/values.h"
#include "chrome/browser/ash/app_list/arc/arc_app_list_prefs.h"
#include "chrome/browser/profiles/profile.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/common/extension_set.h"
#include "fydeos/extensions/common/api/app_management.h"
#include "chrome/browser/ash/arc/arc_util.h"
#include "chrome/browser/policy/chrome_policy_conversions_client.h"
#include "components/policy/core/browser/policy_conversions.h"

namespace extensions{
  const char kAppIdName[] = "appId";
  const char kAppTypeName[] = "appType";
  //const char kPackageName[] = "package";
  const char kWebApp[] = "web_app";
  const char kArcApp[] = "arc_app";
  namespace {
    std::vector<std::string> GetArcAppIdList(content::BrowserContext* context) {
      return ArcAppListPrefs::Get(context)->GetAppIds();
    }

    ExtensionIdSet GetExtensionIdSet(content::BrowserContext* context) {
      return extensions::ExtensionRegistry::Get(context)->enabled_extensions().GetIDs();
    }

    base::Value CreateWebAppInfo(std::string app_id) {
      base::Value::Dict result;
      result.Set(kAppIdName, app_id);
      result.Set(kAppTypeName, kWebApp);
      return base::Value(std::move(result));
    }

    base::Value CreateArcAppInfo(std::string app_id) {
      base::Value::Dict result;
      result.Set(kAppIdName, app_id);
      result.Set(kAppTypeName, kArcApp);
      return base::Value(std::move(result));
    }

  } //exit internel namespace

  ExtensionFunction::ResponseAction AppManagementGetAppListFunction::Run() {
    Profile* profile = Profile::FromBrowserContext(browser_context());
    base::Value::List app_list;
    for(std::string app_id: GetExtensionIdSet(browser_context()))
      app_list.Append(CreateWebAppInfo(app_id));
    if (arc::IsArcAllowedForProfile(profile))
      for(std::string app_id: GetArcAppIdList(browser_context()))
        app_list.Append(CreateArcAppInfo(app_id));
    return RespondNow(WithArguments(std::move(app_list)));
  }

  ExtensionFunction::ResponseAction AppManagementInstallWebAppFunction::Run() {
    return RespondNow(NoArguments());
  }

  ExtensionFunction::ResponseAction AppManagementInstallArcAppFunction::Run() {
    return RespondNow(NoArguments());
  }

  ExtensionFunction::ResponseAction AppManagementGetArcPolicyFunction::Run() {
    auto client = std::make_unique<policy::ChromePolicyConversionsClient>(browser_context());
    base::Value::Dict dict = policy::PolicyConversions(std::move(client)).ToValueDict();
    const base::Value* arc_policy = dict.FindByDottedPath("chromePolicies.ArcPolicy.value");
    std::string policy_value = "";
    if (arc_policy) {
      policy_value = arc_policy->GetString();
    }
    return RespondNow(WithArguments(base::Value(policy_value)));
  }
} //exit namespace extensions
