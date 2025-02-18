#ifndef FYDEOS_TOGGLE_OTA_H_
#define FYDEOS_TOGGLE_OTA_H_

#include "base/functional/callback_forward.h"

namespace fydeos {
namespace misc {
  void EnableFydeOTA(const bool enabled, base::OnceCallback<void()> callback);
  bool GetEnabledFydeOTA();
} // misc

} // fydeos
#endif /* ifndef FYDEOS_TOGGLE_OTA_H_ */
