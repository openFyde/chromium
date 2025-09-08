// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_ASH_LOGIN_SCREENS_EULA_SCREEN_H_
#define CHROME_BROWSER_ASH_LOGIN_SCREENS_EULA_SCREEN_H_

#include <string>

#include "base/functional/callback.h"
#include "chrome/browser/ash/login/screens/base_screen.h"
#include "url/gurl.h"

namespace ash {

class EulaView;

// Representation independent class that controls OOBE screen showing EULA
// to users.
class EulaScreen : public BaseScreen {
 public:
  enum class Result {
    // The user accepted EULA
    ACCEPTED,
    // Eula already accepted, skip screen
    ALREADY_ACCEPTED,
    // The user did not accept EULA - they clicked back button instead.
    BACK,
    // Eula screen is skipped.
    NOT_APPLICABLE,
  };

  // This enum is tied directly to a UMA enum defined in
  // //tools/metrics/histograms/enums.xml, and should always reflect it (do not
  // change one without changing the other).  Entries should be never modified
  // or deleted.  Only additions possible.
  enum class UserAction {
    kAcceptButtonClicked = 0,
    kBackButtonClicked = 1,
    kMaxValue = kBackButtonClicked,
  };

  static std::string GetResultString(Result result);

  using ScreenExitCallback = base::RepeatingCallback<void(Result result)>;
  EulaScreen(base::WeakPtr<EulaView> view,
             const ScreenExitCallback& exit_callback);

  EulaScreen(const EulaScreen&) = delete;
  EulaScreen& operator=(const EulaScreen&) = delete;

  ~EulaScreen() override;

 protected:
  ScreenExitCallback* exit_callback() { return &exit_callback_; }

 private:
  // BaseScreen:
  bool MaybeSkip(WizardContext& context) override;
  void ShowImpl() override;
  void HideImpl() override;
  void OnUserAction(const base::Value::List& args) override;

  base::WeakPtr<EulaView> view_;

  ScreenExitCallback exit_callback_;

  base::WeakPtrFactory<EulaScreen> weak_ptr_factory_{this};
};

}  // namespace ash

#endif  // CHROME_BROWSER_ASH_LOGIN_SCREENS_EULA_SCREEN_H_
