#ifndef _FYDEOS_EXTENSIONS_FYDEOS_ARC_STATUS_FYDEOS_ARC_STATUS_API_H_
#define _FYDEOS_EXTENSIONS_FYDEOS_ARC_STATUS_FYDEOS_ARC_STATUS_API_H_

#include "extensions/browser/extension_function.h"
#include "extensions/browser/extension_function_histogram_value.h"
#include "chrome/browser/ash/arc/process/arc_process_service.h"

namespace extensions {

namespace {

  using OptionalArcProcessList = arc::ArcProcessService::OptionalArcProcessList;

  struct ArcStatus {
    bool supported = false;
    bool archero = false;
    bool running = false;
    bool installable = false;
  };

  using StatusCallback = base::OnceCallback<void(ArcStatus)>;
  class ArcStatusHelper {
    public:
    ArcStatusHelper() = default;
    ~ArcStatusHelper() = default;

    void GetStatus(StatusCallback callback);

    private:
    void GetSystemProcessList();
    void OnGetSystemProcessList(OptionalArcProcessList process_list);
    void GetAppProcessList();
    void OnGetAppProcessList(OptionalArcProcessList process_list);

    void Final();

    StatusCallback callback_;
    ArcStatus status_;

    base::WeakPtrFactory<ArcStatusHelper> weak_factory_{this};
  };
}

  class FydeosArcStatusGetFunction: public ExtensionFunction {
    ~FydeosArcStatusGetFunction() override {}
    ResponseAction Run() override;
    void OnGetStatus(ArcStatus status);
    DECLARE_EXTENSION_FUNCTION("fydeosArcStatus.get", FYDEOS_ARC_STATUS_GET)
  };

}
#endif // _FYDEOS_EXTENSIONS_FYDEOS_ARC_STATUS_FYDEOS_ARC_STATUS_API_H_
