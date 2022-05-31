// Copyright 2022 Fyde Innovations. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview Polymer element for displaying material design fyde local signin
 */

/* #js_imports_placeholder */

/**
 * @constructor
 * @extends {PolymerElement}
 * @implements {LoginScreenBehaviorInterface}
 */
const FydeLocalSigninBase = Polymer.mixinBehaviors(
    [LoginScreenBehavior],
    Polymer.Element);

FydeLocalSigninBase.$;

/**
/* @polymer 
 */
class FydeLocalSignin extends FydeLocalSigninBase {
  static get is() {
    return 'fyde-local-signin-element';
  }

  /* #html_template_placeholder */

  static get properties() {
    return {};
  }

  constructor() {
    super();
  }

  /** @override */
  ready() {
    super.ready();
    this.initializeLoginScreen('FydeLocalSigninScreen', {
      resetAllowed: true,
    });
  }

  onBackClicked() {
    this.userActed('cancel');
  }

}

customElements.define(FydeLocalSignin.is, FydeLocalSignin);
