// Copyright 2020 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fydeos/extensions/browser/api/license_management/license_management_api.h"

#include <stddef.h>
#include "base/functional/bind.h"
#include "base/logging.h"
#include "base/lazy_instance.h"
#include "fydeos/extensions/common/api/license_management.h"

namespace license = extensions::api::license_management;

namespace extensions {
static base::LazyInstance<BrowserContextKeyedAPIFactory<LicenseManagementAPI>>::
  DestructorAtExit g_license_api_factory = LAZY_INSTANCE_INITIALIZER;

// static
BrowserContextKeyedAPIFactory<LicenseManagementAPI>* LicenseManagementAPI::GetFactoryInstance() {
  return g_license_api_factory.Pointer();
}

LicenseManagementAPI* LicenseManagementAPI::Get(content::BrowserContext* context) {
  return GetFactoryInstance()->Get(context);
}

LicenseManagementAPI::LicenseManagementAPI(content::BrowserContext* context)
  : profile_(Profile::FromBrowserContext(context)), weak_factory_(this) {
  RegisterNotifications();
}

LicenseEventRouter::LicenseEventRouter(Profile* profile)
  : license_observer_(this),
    profile_(profile),
    event_router_(EventRouter::Get(profile_)),
    weak_factory_(this){
  CHECK(fydeos::license::LicenseManager::Get() != nullptr);
  license_observer_.Observe(fydeos::license::LicenseManager::Get());
}

LicenseEventRouter::~LicenseEventRouter() = default;

void LicenseEventRouter::AddExtensionId(const std::string& extension_id,
  const std::string& event_name) {
  if (event_name == license::OnValid::kEventName)
    valid_listener_ids_.insert(extension_id);
  else
    invalid_listener_ids_.insert(extension_id);
}

void LicenseEventRouter::RemoveExtensionId(const std::string& extension_id,
  const std::string& event_name) {
  if (event_name == license::OnValid::kEventName)
    valid_listener_ids_.erase(extension_id);
  else
    invalid_listener_ids_.erase(extension_id);
}

void LicenseEventRouter::OnLicenseStateChanged(bool IsValid) {
  base::Value::List event_args;
  if (IsValid) {
    auto event = std::make_unique<Event>(
        events::FYDEOS_LICENSE_VALID_NOTIFICATION,
        license::OnValid::kEventName, std::move(event_args));
    event_router_->BroadcastEvent(std::move(event));
  }else {
    auto event = std::make_unique<Event>(
        events::FYDEOS_LICENSE_INVALID_NOTIFICATION,
        license::OnInvalid::kEventName, std::move(event_args));
    event_router_->BroadcastEvent(std::move(event));
  }
}

void LicenseManagementAPI::RegisterNotifications() {
  VLOG(1) << "Init license management API: register notifications";
  EventRouter* router = EventRouter::Get(profile_);
  router->RegisterObserver(this, license::OnValid::kEventName);
  router->RegisterObserver(this, license::OnInvalid::kEventName);
}

LicenseManagementAPI::~LicenseManagementAPI() {}

void LicenseManagementAPI::ShutDown() {
  EventRouter::Get(profile_)->UnregisterObserver(this);
  license_event_router_.reset();
}

void LicenseManagementAPI::OnListenerAdded(const EventListenerInfo& details) {
  if (!license_event_router_)
    license_event_router_.reset(new LicenseEventRouter(profile_));
  license_event_router_->AddExtensionId(details.extension_id, details.event_name);
}

void LicenseManagementAPI::OnListenerRemoved(const EventListenerInfo& details) {
  license_event_router_->RemoveExtensionId(details.extension_id, details.event_name);
}

ExtensionFunction::ResponseAction LicenseManagementRefreshLicenseFunction::Run() {
  fydeos::license::LicenseManager::Get()->CheckLicense();
  return RespondNow(NoArguments());
}

}  // namespace extensions
