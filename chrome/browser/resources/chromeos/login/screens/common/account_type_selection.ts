// Copyright 2022 Fyde Innovations. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import {html, mixinBehaviors, PolymerElement} from '//resources/polymer/v3_0/polymer/polymer_bundled.min.js';
import {OobeI18nMixin} from '../../components/mixins/oobe_i18n_mixin.js';
import {OobeDialogHostMixin} from '../../components/mixins/oobe_dialog_host_mixin.js';
import '//resources/ash/common/cr_elements/cr_radio_button/cr_card_radio_button.js';
import '//resources/ash/common/cr_elements/cr_radio_group/cr_radio_group.js';
import '../../components/buttons/oobe_back_button.js';
import '../../components/buttons/oobe_next_button.js';
import '../../components/common_styles/oobe_common_styles.css.js';
import '../../components/common_styles/cr_card_radio_group_styles.css.js';
import '../../components/common_styles/oobe_dialog_host_styles.css.js';
import '../../components/dialogs/oobe_adaptive_dialog.js';
import {loadTimeData} from '//resources/ash/common/load_time_data.m.js';
import {getTemplate} from './account_type_selection.html.js';


const AccountTypeSelectionScreenElementBase =
  OobeDialogHostMixin(OobeI18nMixin(PolymerElement));

const AccountTypeToSelect = {
  GOOGLE: 'google',
  FYDE: 'fyde',
};

class AccountTypeSelection extends AccountTypeSelectionScreenElementBase {
  static get is() {
    return 'account-type-selection-element';
  }

  static get template() {
    return getTemplate();
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

  private selectedAccountType_: string;
  private hideBackButton_: boolean;


  override ready() {
    super.ready();
    this.selectedAccountType_ = AccountTypeToSelect.FYDE;
  }


  onBackButtonClicked_() {
    this.dispatchEvent(new CustomEvent('account-type-selection-back'));
  }

  onNextClicked_() {
    this.dispatchEvent(new CustomEvent('account-type-selected', { detail: this.selectedAccountType_ }));
  }

}

customElements.define(AccountTypeSelection.is, AccountTypeSelection);
