// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

(function() {

const UIState = {
  SIGNUP: 'signup',
  SIGNIN: 'signin',
};

/** @enum {number} */ var FYDE_LOCAL_SIGNIN_ERROR_STATE = {
  NONE: 0,
  BAD_USERNAME: 1,
  BAD_AUTH_PASSWORD: 2,
  BAD_CONFIRM_PASSWORD: 3,
  BAD_USERNAME_OR_PASSWORD_ERROR: 4,
};

Polymer({
  is: 'fyde-local-signin-element',

  behaviors: [
    OobeDialogHostBehavior,
    LoginScreenBehavior,
    OobeI18nBehavior,
    MultiStepBehavior,
  ],

  EXTERNAL_API: [
    'reset',
    'setErrorState',
  ],

  properties: {
    loading: {type: Boolean, value: false},
    userRealm: {type: String, value: ''},
    userName: {type: String, value: '', observer: 'userNameObserver_'},
    errorState: {
      type: Number,
      value: FYDE_LOCAL_SIGNIN_ERROR_STATE.NONE,
      observer: 'errorStateObserver_',
    },
    showUsersOnSignin_: {
      type: Boolean,
      value: true,
    },
    showSigninButton_: {
      type: Boolean,
      value: false,
      computed: 'computeShowSigninButton_(showUsersOnSignin_, uiStep)',
    },
    userInvalid:
        {type: Boolean, value: false, observer: 'userInvalidObserver_'},
    authPasswordInvalid:
        {type: Boolean, value: false, observer: 'authPasswordInvalidObserver_'},
    authPasswordConfirmInvalid:
        {type: Boolean, value: false, observer: 'authPasswordConfirmInvalidObserver_'},
    authSigninPasswordInvalid:
        {type: Boolean, value: false, observer: 'authSigninPasswordInvalidObserver_'},
  },

  errorStateLocked_: false,

  defaultUIStep() {
    return UIState.SIGNUP;
  },

  UI_STEPS: UIState,

  ready() {
    this.initializeLoginScreen('FydeLocalSigninScreen', {
      resetAllowed: true,
    });
  },

  onBeforeShow(data) {
    if (data && 'emailDomain' in data) {
      this.userRealm = '@' + data['emailDomain'];
    }
    if (data && 'showUsersOnSignin' in data) {
      this.showUsersOnSignin_ = data.showUsersOnSignin;
    }
    this.focus();
  },

  setErrorState(username, errorState) {
    this.userName = username;
    this.errorState = errorState;
    this.loading = false;
  },

  reset() {
    this.$.userInput.value = '';
    this.$.passwordInput.value = '';
    this.$.passwordConfirmInput.value = '';
    this.$.signinPasswordInput.value = '';
    this.errorState = FYDE_LOCAL_SIGNIN_ERROR_STATE.NONE;
    this.loading = false;
  },

  focus() {
    if (!this.$.userInput.value) {
      this.$.userInput.focus();
    } else {
      this.$.passwordInput.focus();
    }
  },

  errorStateObserver_() {
    if (this.errorStateLocked_)
      return;

    this.errorStateLocked_ = true;

    this.userInvalid = this.errorState === FYDE_LOCAL_SIGNIN_ERROR_STATE.BAD_USERNAME;
    this.authPasswordInvalid = this.errorState === FYDE_LOCAL_SIGNIN_ERROR_STATE.BAD_AUTH_PASSWORD;
    this.authPasswordConfirmInvalid = this.errorState === FYDE_LOCAL_SIGNIN_ERROR_STATE.BAD_CONFIRM_PASSWORD;
    this.authSigninPasswordInvalid = this.errorState === FYDE_LOCAL_SIGNIN_ERROR_STATE.BAD_USERNAME_OR_PASSWORD_ERROR;

    this.errorStateLocked_ = false;
  },

  onSubmit_() {
    this.userInvalid = !this.$.userInput.validate();
    if (this.userInvalid)
      return;
    if (this.uiStep === UIState.SIGNUP) {
      this.authPasswordInvalid = !this.$.passwordInput.validate();
      if (this.authPasswordInvalid)
        return;
      this.authPasswordConfirmInvalid = !this.$.passwordConfirmInput.validate() || (this.$.passwordInput.value !== this.$.passwordConfirmInput.value);
      if (this.authPasswordConfirmInvalid)
        return;
    } else if (this.uiStep === UIState.SIGNIN) {
      this.authSigninPasswordInvalid = !this.$.signinPasswordInput.validate();
      if (this.authSigninPasswordInvalid)
        return;
    } else {
      return;
    }

    var user = /** @type {string} */ (this.$.userInput.value);
    if (!user.includes('@') && this.userRealm)
      user += this.userRealm;
    let username = user;
    let password = '';
    let newUser = this.uiStep === UIState.SIGNUP;
    if (newUser) {
      password = this.$.passwordInput.value;
    } else {
      password = this.$.signinPasswordInput.value;
    }
    var msg = {
      newUser,
      username,
      password,
    };
    this.loading = true;
    chrome.send('completeFtAuthentication', [msg.newUser, msg.username, msg.password]);
  },

  onBackButton_() {
    this.userActed('cancel');
  },

  onKeydownUserInput_(e) {
    this.errorState = FYDE_LOCAL_SIGNIN_ERROR_STATE.NONE;
    if (e.key == 'Enter') {
      if (this.uiStep === UIState.SIGNUP) {
        this.switchTo_('passwordInput');
      } else if (this.uiStep === UIState.SIGNIN) {
        this.switchTo_('signinPasswordInput');
      }
    }
  },

  userNameObserver_() {
    if (this.userRealm && this.userName &&
        this.userName.endsWith(this.userRealm)) {
      this.userName = this.userName.replace(this.userRealm, '');
    }
  },

  onKeydownAuthPasswordInput_(e) {
    this.errorState = FYDE_LOCAL_SIGNIN_ERROR_STATE.NONE;
    if (e.key == 'Enter')
      this.switchTo_('passwordConfirmInput') || this.onSubmit_();
  },

  onKeydownAuthPasswordConfirmInput_(e) {
    this.authPasswordConfirmInvalid = false;
    if (e.key == 'Enter')
      this.onSubmit_();
  },

  onKeydownAuthSigninPasswordInput_(e) {
    this.errorState = FYDE_LOCAL_SIGNIN_ERROR_STATE.NONE;
    if (e.key == 'Enter')
      this.onSubmit_();
  },

  switchTo_(inputId) {
    if (!this.$[inputId].disabled && this.$[inputId].value.length == 0) {
      this.$[inputId].focus();
      return true;
    }
    return false;
  },

  userInvalidObserver_(isInvalid) {
    this.setErrorState_(isInvalid, FYDE_LOCAL_SIGNIN_ERROR_STATE.BAD_USERNAME);
  },

  authPasswordInvalidObserver_(isInvalid) {
    this.setErrorState_(
        isInvalid, FYDE_LOCAL_SIGNIN_ERROR_STATE.BAD_AUTH_PASSWORD);
  },

  authPasswordConfirmInvalidObserver_(isInvalid) {
    this.setErrorState_(
        isInvalid, FYDE_LOCAL_SIGNIN_ERROR_STATE.BAD_CONFIRM_PASSWORD);
  },

  authSigninPasswordInvalidObserver_(isInvalid) {
    this.setErrorState_(
        isInvalid, FYDE_LOCAL_SIGNIN_ERROR_STATE.BAD_USERNAME_OR_PASSWORD_ERROR);
  },

  setErrorState_(isInvalid, error) {
    if (this.errorStateLocked_)
      return;
    this.errorStateLocked_ = true;

    if (isInvalid)
      this.errorState = error;
    else
      this.errorState = FYDE_LOCAL_SIGNIN_ERROR_STATE.NONE;

    this.errorStateLocked_ = false;
  },

  onGotoSignupClicked_() {
    this.reset();
    this.setUIStep(UIState.SIGNUP);
  },

  onGotoSigninClicked_() {
    this.reset();
    this.setUIStep(UIState.SIGNIN);
  },

  computeShowSigninButton_(showUsersOnSignin, uiStep) {
    return !showUsersOnSignin && uiStep === UIState.SIGNUP;
  },

});

})();
