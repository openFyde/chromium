// Copyright 2020 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LICENSE_MANAGEMENT_API_H_
#define LICENSE_MANAGEMENT_API_H_

#include <string>
#include "base/memory/weak_ptr.h"
#include "base/scoped_observation.h"
#include "extensions/browser/browser_context_keyed_api_factory.h"
#include "chrome/browser/profiles/profile.h"
#include "extensions/browser/event_router.h"
#include "extensions/browser/extension_function.h"
#include "extensions/browser/extension_function_histogram_value.h"
#include "fydeos/license/fydeos_license_manager.h"

using LicenseManager = fydeos::license::LicenseManager;
using LicenseObserver = LicenseManager::Observer;

namespace extensions {
class LicenseEventRouter: public LicenseObserver {
 public:
  explicit LicenseEventRouter(Profile* profile);
  void AddExtensionId(const std::string& extension_id, const std::string& event_name);
  void RemoveExtensionId(const std::string& extension_id, const std::string& event_name);
  void OnLicenseStateChanged(bool IsValid) override;
  ~LicenseEventRouter() override;
 private:
  base::ScopedObservation<LicenseManager, LicenseObserver> license_observer_;
  Profile* profile_;
  EventRouter* event_router_; 
  std::set<std::string> invalid_listener_ids_;
  std::set<std::string> valid_listener_ids_;
  base::WeakPtrFactory<LicenseEventRouter> weak_factory_;
}; // class LicenseEventRouter

class LicenseManagementAPI: public BrowserContextKeyedAPI,
                            public EventRouter::Observer {
 public:
  static BrowserContextKeyedAPIFactory<LicenseManagementAPI>* GetFactoryInstance();
  static LicenseManagementAPI* Get(content::BrowserContext* context);
  explicit LicenseManagementAPI(content::BrowserContext* context);
  ~LicenseManagementAPI() override;
  void ShutDown();

  // EventRouter::Observer implementation.
  void OnListenerAdded(const EventListenerInfo& details) override;
  void OnListenerRemoved(const EventListenerInfo& details) override;
 private:
   friend class BrowserContextKeyedAPIFactory<LicenseManagementAPI>;
    static const char* service_name() { return "LicenseManageAPI"; }
    static const bool kServiceRedirectedInIncognito = true;
    static const bool kServiceIsNULLWhileTesting = true;
    void RegisterNotifications();
    Profile* profile_;
    std::unique_ptr<LicenseEventRouter> license_event_router_;
    base::WeakPtrFactory<LicenseManagementAPI> weak_factory_;
}; // class LicenseManagementAPI

class LicenseManagementRefreshLicenseFunction: public ExtensionFunction {
  ~LicenseManagementRefreshLicenseFunction() override {}
  ResponseAction Run() override;
  DECLARE_EXTENSION_FUNCTION("licenseManagement.refreshLicense",LICENSE_MANAGEMENT_REFRESH)
};
}  // namespace extensions



#endif  // LICENSE_MANAGEMENT_API_H_
