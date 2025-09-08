// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_CHROMEOS_LOGIN_EULA_SCREEN_HANDLER_H_
#define CHROME_BROWSER_UI_WEBUI_CHROMEOS_LOGIN_EULA_SCREEN_HANDLER_H_

#include <string>

#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "base/values.h"
#include "chrome/browser/ui/webui/ash/login/base_screen_handler.h"


namespace ash {

// Interface between eula screen and its representation, either WebUI
// or Views one. Note, do not forget to call OnViewDestroyed in the
// dtor.
class EulaView {
 public:
  inline constexpr static StaticOobeScreenId kScreenId{"oobe-eula-md",
                                                       "EulaScreen"};

  virtual ~EulaView() = default;

  virtual void Show(const bool is_cloud_ready_update_flow) = 0;
  virtual void Hide() = 0;
  virtual base::WeakPtr<EulaView> AsWeakPtr() = 0;
};

// WebUI implementation of EulaScreenView. It is used to interact
// with the eula part of the JS page.
class EulaScreenHandler : public EulaView, public BaseScreenHandler {
 public:
  using TView = EulaView;

  EulaScreenHandler();

  EulaScreenHandler(const EulaScreenHandler&) = delete;
  EulaScreenHandler& operator=(const EulaScreenHandler&) = delete;

  ~EulaScreenHandler() override;

  // EulaView implementation:
  void Show(const bool is_cloud_ready_update_flow) override;
  void Hide() override;
  base::WeakPtr<EulaView> AsWeakPtr() override;

  // BaseScreenHandler implementation:
  void DeclareLocalizedValues(
      ::login::LocalizedValuesBuilder* builder) override;

 private:
  // Determines the online URL to use.
  std::string GetEulaOnlineUrl();
  std::string GetPrivacyOnlineUrl();

  base::WeakPtrFactory<EulaScreenHandler> weak_ptr_factory_{this};
};

}  // namespace ash

#endif  // CHROME_BROWSER_UI_WEBUI_CHROMEOS_LOGIN_EULA_SCREEN_HANDLER_H_
