// Copyright 2022 Fyde Innovations. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview Polymer element for displaying material design fyde local signin
 */

import {html, mixinBehaviors, PolymerElement} from '//resources/polymer/v3_0/polymer/polymer_bundled.min.js';
import {LoginScreenBehavior, LoginScreenBehaviorInterface} from '../../components/behaviors/login_screen_behavior.m.js';

/**
 * @constructor
 * @extends {PolymerElement}
 * @implements {LoginScreenBehaviorInterface}
 */
const FydeLocalSigninBase = mixinBehaviors(
    [LoginScreenBehavior],
    PolymerElement);

FydeLocalSigninBase.$;

/**
/* @polymer 
 */
class FydeLocalSignin extends FydeLocalSigninBase {
  static get is() {
    return 'fyde-local-signin-element';
  }

  static get template() {
    return html`{__html_template__}`;
  }

  static get properties() {
    return {};
  }

  constructor() {
    super();
  }

  /** @override */
  ready() {
    super.ready();
    this.initializeLoginScreen('FydeLocalSigninScreen');
  }

  onBackClicked() {
    this.userActed('cancel');
  }

}

customElements.define(FydeLocalSignin.is, FydeLocalSignin);
