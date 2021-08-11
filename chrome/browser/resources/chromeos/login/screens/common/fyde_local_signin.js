// Copyright 2022 Fyde Innovations. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview Polymer element for displaying material design fyde local signin
 */

/* #js_imports_placeholder */

/**
 * @enum {number}
 */
/* #export */ const FYDE_LOCAL_SIGNIN_ERROR_STATE = {
  NONE: 0,
  BAD_USERNAME: 1,
  BAD_AUTH_PASSWORD: 2,
  BAD_CONFIRM_PASSWORD: 3,
};

/**
 * @constructor
 * @extends {PolymerElement}
 * @implements {LoginScreenBehaviorInterface}
 * @implements {OobeI18nBehaviorInterface}
 * @implements {OobeDialogBehaviorInterface}
 */
const FydeLocalSigninBase = Polymer.mixinBehaviors(
  [OobeDialogHostBehavior, LoginScreenBehavior, OobeI18nBehavior,],
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
    return {
      loading: {type: Boolean, value: false},
      userRealm: {type: String, value: ''},
      userName: {type: String, value: '', observer: 'userNameObserver_'},
      errorState: {
        type: Number,
        value: FYDE_LOCAL_SIGNIN_ERROR_STATE.NONE,
        observer: 'errorStateObserver_',
      },
      userInvalid:
          {type: Boolean, value: false, observer: 'userInvalidObserver_'},
      authPasswordInvalid:
          {type: Boolean, value: false, observer: 'authPasswordInvalidObserver_'},
      authPasswordConfirmInvalid:
          {type: Boolean, value: false, observer: 'authPasswordConfirmInvalidObserver_'},
    };
  }

  constructor() {
    super();
    this.errorStateLocked_ = false;
  }

  get EXTERNAL_API() {
    return ['reset', 'setErrorState'];
  }

  /** @override */
  ready() {
    super.ready();
    this.initializeLoginScreen('FydeLocalSigninScreen', {
      resetAllowed: true,
    });
  }

  onBeforeShow(data) {
    if (data && 'emailDomain' in data) {
      this.userRealm = '@' + data['emailDomain'];
    }
    this.focus();
  }

  setErrorState(username, errorState) {
    this.userName = username;
    this.errorState = errorState;
    this.loading = false;
  }

  reset() {
    this.$.userInput.value = '';
    this.$.passwordInput.value = '';
    this.$.passwordConfirmInput.value = '';
    this.errorState = FYDE_LOCAL_SIGNIN_ERROR_STATE.NONE;
  }

  focus() {
    if (!this.$.userInput.value) {
      this.$.userInput.focus();
    } else {
      this.$.passwordInput.focus();
    }
  }

  errorStateObserver_() {
    if (this.errorStateLocked_)
      return;

    this.errorStateLocked_ = true;

    this.userInvalid = this.errorState === FYDE_LOCAL_SIGNIN_ERROR_STATE.BAD_USERNAME;
    this.authPasswordInvalid = this.errorState === FYDE_LOCAL_SIGNIN_ERROR_STATE.BAD_AUTH_PASSWORD;
    this.authPasswordConfirmInvalid = this.errorState === FYDE_LOCAL_SIGNIN_ERROR_STATE.BAD_CONFIRM_PASSWORD;

    this.errorStateLocked_ = false;
  }

  onSubmit_() {
    this.userInvalid = !this.$.userInput.validate();
    if (this.userInvalid)
      return;
    this.authPasswordInvalid = !this.$.passwordInput.validate();
    if (this.authPasswordInvalid)
      return;
    this.authPasswordConfirmInvalid = !this.$.passwordConfirmInput.validate() || (this.$.passwordInput.value !== this.$.passwordConfirmInput.value);
    if (this.authPasswordConfirmInvalid)
      return;

    var user = /** @type {string} */ (this.$.userInput.value);
    if (!user.includes('@') && this.userRealm)
      user += this.userRealm;
    var msg = {
      'username': user,
      'password': this.$.passwordInput.value,
    };
    this.loading = true;
    chrome.send('completeFtAuthentication', [msg.username, msg.password]);
  }

  onBackButton_() {
    this.userActed('cancel');
  }

  onKeydownUserInput_(e) {
    this.errorState = FYDE_LOCAL_SIGNIN_ERROR_STATE.NONE;
    if (e.key == 'Enter')
      this.switchTo_('passwordInput');
  }

  userNameObserver_() {
    if (this.userRealm && this.userName &&
        this.userName.endsWith(this.userRealm)) {
      this.userName = this.userName.replace(this.userRealm, '');
    }
  }

  onKeydownAuthPasswordInput_(e) {
    this.errorState = FYDE_LOCAL_SIGNIN_ERROR_STATE.NONE;
    if (e.key == 'Enter')
      this.switchTo_('passwordConfirmInput') || this.onSubmit_();
  }

  onKeydownAuthPasswordConfirmInput_(e) {
    this.authPasswordConfirmInvalid = false;
    if (e.key == 'Enter')
      this.onSubmit_();
  }

  switchTo_(inputId) {
    if (!this.$[inputId].disabled && this.$[inputId].value.length == 0) {
      this.$[inputId].focus();
      return true;
    }
    return false;
  }

  userInvalidObserver_(isInvalid) {
    this.setErrorState_(isInvalid, FYDE_LOCAL_SIGNIN_ERROR_STATE.BAD_USERNAME);
  }

  authPasswordInvalidObserver_(isInvalid) {
    this.setErrorState_(
        isInvalid, FYDE_LOCAL_SIGNIN_ERROR_STATE.BAD_AUTH_PASSWORD);
  }

  authPasswordConfirmInvalidObserver_(isInvalid) {
    this.setErrorState_(
        isInvalid, FYDE_LOCAL_SIGNIN_ERROR_STATE.BAD_CONFIRM_PASSWORD);
  }

  setErrorState_(isInvalid, error) {
    if (this.errorStateLocked_)
      return;
    this.errorStateLocked_ = true;

    if (isInvalid)
      this.errorState = error;
    else
      this.errorState = FYDE_LOCAL_SIGNIN_ERROR_STATE.NONE;

    this.errorStateLocked_ = false;
  }
}

customElements.define(FydeLocalSignin.is, FydeLocalSignin);
