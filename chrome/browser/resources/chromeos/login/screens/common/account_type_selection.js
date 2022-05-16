// Copyright 2022 Fyde Innovations. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


/* #js_imports_placeholder */

/**
 * @constructor
 * @extends {PolymerElement}
 * @implements {LoginScreenBehaviorInterface}
 * @implements {MultiStepBehaviorInterface}
 */

const AccountTypeSelectionScreenElementBase = Polymer.mixinBehaviors(
  [
    OobeI18nBehavior, OobeDialogHostBehavior
  ],
  Polymer.Element);

const AccountTypeToSelect = {
  GOOGLE: 'google',
  FYDE: 'fyde',
};

AccountTypeSelectionScreenElementBase.$;

class AccountTypeSelection extends AccountTypeSelectionScreenElementBase {
  static get is() {
    return 'account-type-selection-element';
  }

  static get properties() {
    return {
      selectedAccountType_: {
        type: String,
      },
      hideBackButton_: {
        type: Boolean,
        value: () => {
          return Oobe.getInstance().isOobeUI();
        },
      },
    }
  }

  constructor() {
    super();
  }

  ready() {
    super.ready();
    console.log('account-type-selection ready');
    this.selectedAccountType_ = AccountTypeToSelect.FYDE;
  }

  attached() {
    super.attached();
    console.log('account-type-selection attached');
  }

  onBeforeShow() {
    console.log('account type selection onBeforeShow');
  }

  onBackButtonClicked_() {
    this.fire('account-type-selection-back');
  }

  onNextClicked_() {
    this.fire('account-type-selected', this.selectedAccountType_);
  }

}

customElements.define(AccountTypeSelection.is, AccountTypeSelection);
