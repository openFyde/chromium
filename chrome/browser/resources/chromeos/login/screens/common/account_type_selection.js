// Copyright 2022 Fyde Innovations. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import {html, mixinBehaviors, PolymerElement} from '//resources/polymer/v3_0/polymer/polymer_bundled.min.js';
import {OobeI18nBehavior, OobeI18nBehaviorInterface} from '../../components/behaviors/oobe_i18n_behavior.js';
import {LoginScreenBehavior, LoginScreenBehaviorInterface} from '../../components/behaviors/login_screen_behavior.js';
import {OobeDialogHostBehavior} from '../../components/behaviors/oobe_dialog_host_behavior.js';
import '//resources/cr_elements/cr_radio_button/cr_card_radio_button.js';
import '//resources/cr_elements/cr_radio_group/cr_radio_group.js';
import '../../components/buttons/oobe_back_button.js';
import '../../components/buttons/oobe_next_button.js';
import '../../components/common_styles/oobe_common_styles.css.js';
import '../../components/common_styles/cr_card_radio_group_styles.css.js';
import '../../components/common_styles/oobe_dialog_host_styles.css.js';
import '../../components/dialogs/oobe_adaptive_dialog.js';
import {loadTimeData} from '//resources/ash/common/load_time_data.m.js';

/**
 * @constructor
 * @extends {PolymerElement}
 * @implements {LoginScreenBehaviorInterface}
 * @implements {MultiStepBehaviorInterface}
 */

const AccountTypeSelectionScreenElementBase = mixinBehaviors(
  [
    OobeI18nBehavior, OobeDialogHostBehavior
  ],
  PolymerElement);

const AccountTypeToSelect = {
  GOOGLE: 'google',
  FYDE: 'fyde',
};

AccountTypeSelectionScreenElementBase.$;

/**
 * @polymer
 */
class AccountTypeSelection extends AccountTypeSelectionScreenElementBase {
  static get is() {
    return 'account-type-selection-element';
  }

  static get template() {
    return html`{__html_template__}`;
  }

  static get properties() {
    return {
      selectedAccountType_: {
        type: String,
      },
      hideBackButton_: {
        type: Boolean,
        value: () => {
          return loadTimeData.getBoolean('isOobeFlow');
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
