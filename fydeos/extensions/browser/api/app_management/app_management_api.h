#ifndef FLINTOS_EXTENSIONS_API_APP_MANAGEMENT_API_H_
#define FLINTOS_EXTENSIONS_API_APP_MANAGEMENT_API_H_
#include <string>
#include <vector>
#include "extensions/browser/extension_function.h"
#include "extensions/browser/extension_function_histogram_value.h"

namespace extensions {

  class AppManagementGetAppListFunction: public ExtensionFunction {
    ~AppManagementGetAppListFunction() override {}
    ResponseAction Run() override;
    DECLARE_EXTENSION_FUNCTION("appManagement.getAppList", APP_MANAGEMENT_GETAPPLIST)
  };

  class AppManagementInstallWebAppFunction: public ExtensionFunction {
    ~AppManagementInstallWebAppFunction() override {}
    ResponseAction Run() override;
    DECLARE_EXTENSION_FUNCTION("appManagement.installWebApp", APP_MANAGEMENT_INSTALLWEBAPP)
  };

  class AppManagementInstallArcAppFunction: public ExtensionFunction {
    ~AppManagementInstallArcAppFunction() override {}
    ResponseAction Run() override;
    DECLARE_EXTENSION_FUNCTION("appManagement.installArcApp", APP_MANAGEMENT_INSTALLARCAPP)
  };

  class AppManagementGetArcPolicyFunction: public ExtensionFunction {
    ~AppManagementGetArcPolicyFunction() override {}
    ResponseAction Run() override;
    DECLARE_EXTENSION_FUNCTION("appManagement.getArcPolicy", APP_MANAGEMENT_GETARCPOLICY)
  };
}
#endif // FYDEOS_EXTENSIONS_API_APP_MANAGEMENT_API_H_
