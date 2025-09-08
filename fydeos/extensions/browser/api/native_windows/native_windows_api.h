#ifndef FLINTOS_EXTENSIONS_API_TABS_TABS_API_H_
#define FLINTOS_EXTENSIONS_API_TABS_TABS_API_H_

#include <string>
#include <vector>
#include "extensions/browser/extension_function.h"
#include "extensions/browser/extension_function_histogram_value.h"

namespace extensions {

class NativeWindowsGetFunction : public ExtensionFunction {
  ~NativeWindowsGetFunction() override {}
  ResponseAction Run() override;
  DECLARE_EXTENSION_FUNCTION("nativeWindows.get", NATIVE_WINDOWS_GET)
};
class NativeWindowsGetAllFunction : public ExtensionFunction {
  ~NativeWindowsGetAllFunction() override {}
  ResponseAction Run() override;
  DECLARE_EXTENSION_FUNCTION("nativeWindows.getAll", NATIVE_WINDOWS_GETALL)
};
class NativeWindowsUpdateFunction : public ExtensionFunction {
  ~NativeWindowsUpdateFunction() override {}
  ResponseAction Run() override;
  DECLARE_EXTENSION_FUNCTION("nativeWindows.update", NATIVE_WINDOWS_UPDATE)
};
class NativeWindowsRemoveFunction : public ExtensionFunction {
  ~NativeWindowsRemoveFunction() override {}
  ResponseAction Run() override;
  DECLARE_EXTENSION_FUNCTION("nativeWindows.remove", NATIVE_WINDOWS_REMOVE)
};
class NativeWindowsCreateFunction : public ExtensionFunction {
  ~NativeWindowsCreateFunction() override {}
  ResponseAction Run() override;
  DECLARE_EXTENSION_FUNCTION("nativeWindows.create", NATIVE_WINDOWS_CREATE)
};

}

#endif  // FYDEOS_EXTENSIONS_API_TABS_TABS_API_H_
