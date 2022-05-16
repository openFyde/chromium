// Copyright 2022 Fyde Innovations. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

(function() {

const AccountTypeToSelect = {
  GOOGLE: 'google',
  FYDE: 'fyde',
};

Polymer({

  is: 'account-type-selection-element',
  behaviors: [OobeI18nBehavior, OobeDialogHostBehavior],
  properties: {
    selectedAccountType_: {
      type: String,
    },
    hideBackButton_: {
      type: Boolean,
      value: () => {
        return Oobe.getInstance().isOobeUI();
      },
    },
  },

  ready() {
    console.log('account-type-selection ready');
    this.selectedAccountType_ = AccountTypeToSelect.FYDE;
  },

  attached() {
    console.log('account-type-selection attached');
  },

  onBackButtonClicked_() {
    this.fire('account-type-selection-back');
  },

  onNextClicked_() {
    this.fire('account-type-selected', this.selectedAccountType_);
  },

});
})();
