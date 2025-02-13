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
import {OobeDialogHostMixin} from '../../components/mixins/oobe_dialog_host_mixin.js';

import {CrInputElement} from '//resources/ash/common/cr_elements/cr_input/cr_input.js';
import {assert} from '//resources/js/assert.js';

import '//resources/ash/common/cr_elements/icons.html.js';
import '//resources/polymer/v3_0/paper-progress/paper-progress.js';
import '//resources/polymer/v3_0/iron-icon/iron-icon.js';
import '../../components/oobe_icons.html.js';
import '../../components/buttons/oobe_back_button.js';
import '../../components/buttons/oobe_next_button.js';
import '../../components/buttons/oobe_text_button.js';
import '../../components/common_styles/oobe_common_styles.css.js';
import '../../components/common_styles/oobe_dialog_host_styles.css.js';
import '../../components/dialogs/oobe_adaptive_dialog.js';

import {getTemplate} from './fyde_local_signin.html.js';

enum FYDE_LOCAL_SIGNIN_ERROR_STATE {
  NONE = 0,
  BAD_USERNAME = 1,
  BAD_AUTH_PASSWORD = 2,
  BAD_CONFIRM_PASSWORD = 3,
};

interface FydeLocalSigninData {
  emailDomain: string;
}

const FydeLocalSigninBase = OobeDialogHostMixin(LoginScreenMixin(OobeI18nMixin(PolymerElement)));

export class FydeLocalSignin extends FydeLocalSigninBase {
  static get is() {
    return 'fyde-local-signin-element' as const;
  }

  static get template() {
    return getTemplate();
  }

  static get properties(): PolymerElementProperties {
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

  private loading: boolean;
  private errorStateLocked_: boolean;
  private userName: string;
  private userRealm: string;
  private errorState: FYDE_LOCAL_SIGNIN_ERROR_STATE;
  private userInvalid: boolean;
  private authPasswordInvalid: boolean;
  private authPasswordConfirmInvalid: boolean;
  private passwordInput: CrInputElement;
  private passwordConfirmInput: CrInputElement;
  private userInput: CrInputElement;

  constructor() {
    super();
    this.errorStateLocked_ = false;
  }

  override get EXTERNAL_API(): string[] {
    return ['reset', 'setErrorState'];
  }

  override ready() {
    super.ready();
    this.initializeLoginScreen('FydeLocalSigninScreen');

    const passwordInput =
        this.shadowRoot?.querySelector<CrInputElement>('#passwordInput');
    assert(passwordInput instanceof CrInputElement);
    this.passwordInput = passwordInput;

    const passwordConfirmInput =
        this.shadowRoot?.querySelector<CrInputElement>('#passwordConfirmInput');
    assert(passwordConfirmInput instanceof CrInputElement);
    this.passwordConfirmInput = passwordConfirmInput;

    const userInput =
        this.shadowRoot?.querySelector<CrInputElement>('#userInput');
    assert(userInput instanceof CrInputElement);
    this.userInput = userInput;
  }

  override onBeforeShow(data: FydeLocalSigninData) {
    if (data && 'emailDomain' in data) {
      this.userRealm = '@' + data['emailDomain'];
    }
    this.focus_();
  }

  setErrorState(username: string, errorState: FYDE_LOCAL_SIGNIN_ERROR_STATE) {
    this.userName = username;
    this.errorState = errorState;
    this.loading = false;
  }

  reset() {
    this.userInput.value = '';
    this.passwordInput.value = '';
    this.passwordConfirmInput.value = '';
    this.errorState = FYDE_LOCAL_SIGNIN_ERROR_STATE.NONE;
  }

  focus_() {
    if (!this.userInput.value) {
      this.userInput.focus();
    } else {
      this.passwordInput.focus();
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
    this.userInvalid = /^[a-zA-Z][a-zA-Z0-9\.\-]*$/.test(this.userInput.value) === false;
    if (this.userInvalid)
      return;
    this.authPasswordInvalid = !this.passwordInput.validate();
    if (this.authPasswordInvalid)
      return;
    this.authPasswordConfirmInvalid = !this.passwordConfirmInput.validate() || (this.passwordInput.value !== this.passwordConfirmInput.value);
    if (this.authPasswordConfirmInvalid)
      return;

    var user = /** @type {string} */ (this.userInput.value);
    if (!user.includes('@') && this.userRealm)
      user += this.userRealm;
    var msg = {
      'username': user,
      'password': this.passwordInput.value,
    };
    this.loading = true;
    chrome.send('completeFtAuthentication', [msg.username, msg.password]);
  }

  onBackButton_() {
    this.userActed('cancel');
  }

  onKeydownUserInput_(e: KeyboardEvent) {
    this.errorState = FYDE_LOCAL_SIGNIN_ERROR_STATE.NONE;
    if (e.key == 'Enter')
      this.switchTo_(this.passwordInput);
  }

  userNameObserver_() {
    if (this.userRealm && this.userName &&
        this.userName.endsWith(this.userRealm)) {
      this.userName = this.userName.replace(this.userRealm, '');
    }
  }

  onKeydownAuthPasswordInput_(e: KeyboardEvent) {
    this.errorState = FYDE_LOCAL_SIGNIN_ERROR_STATE.NONE;
    if (e.key == 'Enter')
      this.switchTo_(this.passwordConfirmInput) || this.onSubmit_();
  }

  onKeydownAuthPasswordConfirmInput_(e: KeyboardEvent) {
    this.authPasswordConfirmInvalid = false;
    if (e.key == 'Enter')
      this.onSubmit_();
  }

  switchTo_(ele: CrInputElement) {
    if (!ele.disabled && ele.value.length == 0) {
      ele.focus();
      return true;
    }
    return false;
  }

  userInvalidObserver_(isInvalid: boolean) {
    this.setErrorState_(isInvalid, FYDE_LOCAL_SIGNIN_ERROR_STATE.BAD_USERNAME);
  }

  authPasswordInvalidObserver_(isInvalid: boolean) {
    this.setErrorState_(
        isInvalid, FYDE_LOCAL_SIGNIN_ERROR_STATE.BAD_AUTH_PASSWORD);
  }

  authPasswordConfirmInvalidObserver_(isInvalid: boolean) {
    this.setErrorState_(
        isInvalid, FYDE_LOCAL_SIGNIN_ERROR_STATE.BAD_CONFIRM_PASSWORD);
  }

  setErrorState_(isInvalid: boolean, error: FYDE_LOCAL_SIGNIN_ERROR_STATE) {
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

declare global {
  interface HTMLElementTagNameMap {
    [FydeLocalSignin.is]: FydeLocalSignin;
  }
}

customElements.define(FydeLocalSignin.is, FydeLocalSignin);
