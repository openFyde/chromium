// Copyright 2022 Fyde Innovations. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview Polymer element for displaying material design fyde local signin
 */

import {PolymerElementProperties} from '//resources/polymer/v3_0/polymer/interfaces.js';
import {PolymerElement} from '//resources/polymer/v3_0/polymer/polymer_bundled.min.js';
import {LoginScreenMixin} from '../../components/mixins/login_screen_mixin.js';
import {OobeI18nMixin} from '../../components/mixins/oobe_i18n_mixin.js';

import {getTemplate} from './fyde_local_signin.html.js';

const FydeLocalSigninBase = LoginScreenMixin(OobeI18nMixin(PolymerElement));

export class FydeLocalSignin extends FydeLocalSigninBase {
  static get is() {
    return 'fyde-local-signin-element' as const;
  }

  static get template() {
    return getTemplate();
  }

  static get properties(): PolymerElementProperties {
    return {};
  }

  constructor() {
    super();
  }

  override ready() {
    super.ready();
    this.initializeLoginScreen('FydeLocalSigninScreen');
  }

  onBackClicked() {
    this.userActed('cancel');
  }

}

declare global {
  interface HTMLElementTagNameMap {
    [FydeLocalSignin.is]: FydeLocalSignin;
  }
}

customElements.define(FydeLocalSignin.is, FydeLocalSignin);
