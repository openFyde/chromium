// Copyright 2016 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import '//resources/ash/common/cr_elements/cr_shared_style.css.js';
import '//resources/ash/common/cr_elements/cr_checkbox/cr_checkbox.js';
import '//resources/js/action_link.js';
import '//resources/polymer/v3_0/iron-icon/iron-icon.js';
import '../../components/oobe_icons.html.js';
import '../../components/common_styles/oobe_common_styles.css.js';
import '../../components/common_styles/oobe_dialog_host_styles.css.js';
import '../../components/dialogs/oobe_adaptive_dialog.js';
import '../../components/dialogs/oobe_modal_dialog.js';

import {assert} from '//resources/js/assert.js';
import {loadTimeData} from '//resources/js/load_time_data.js';
import {afterNextRender, html, mixinBehaviors, PolymerElement} from '//resources/polymer/v3_0/polymer/polymer_bundled.min.js';

import {LoginScreenMixin} from '../../components/mixins/login_screen_mixin.js';
import {OobeI18nMixin} from '../../components/mixins/oobe_i18n_mixin.js';
import {OobeDialogHostMixin} from '../../components/mixins/oobe_dialog_host_mixin.js';
import {MultiStepMixin} from '../../components/mixins/multi_step_mixin.js';
import '../../components/buttons/oobe_back_button.js';
import '../../components/buttons/oobe_text_button.js';
import {WebViewHelper, ContentType} from '../../components/web_view_helper.js';
import {WebViewLoader} from '../../components/web_view_loader.js';
import {Oobe} from '../../cr_ui.js';

import {getTemplate} from './oobe_eula.html.js';

const EulaScreenState = {
  LOADING: 'loading',
  EULA: 'eula',
  PRIVACY: 'privacy',
};

const ONLINE_EULA_LOAD_TIMEOUT_IN_MS = 10000;

const EULA_TERMS_URL = 'chrome://terms';

const EulaScreenBase = OobeDialogHostMixin(LoginScreenMixin(MultiStepMixin(OobeI18nMixin(PolymerElement))));

class EulaScreen extends EulaScreenBase {
  static get is() {
    return 'oobe-eula-element';
  }

  static get template() {
    return getTemplate();
  }

  static get properties() {
    return {
      initialized_: {
        type: Boolean,
      },

      backButtonHidden_: {
        type: Boolean,
      },
    };
  }

  private initialized_: boolean;
  private backButtonHidden_: boolean;
  private offline_: boolean;

  constructor() {
    super();
    this.initialized_ = false;
    this.backButtonHidden_ = false;
    this.offline_ = false;
  }

  override defaultUIStep() {
    return EulaScreenState.LOADING;
  }

  override get UI_STEPS() {
    return EulaScreenState;
  }

  override onBeforeShow(data: { backButtonHidden: boolean }) {
    super.onBeforeShow(data);
    if (data && 'backButtonHidden' in data) {
      this.backButtonHidden_ = data['backButtonHidden'];
    }
    this.updateLocalizedContent();
    window.setTimeout(this.initializeScreen_.bind(this), 0);
    this.loadEula();
  }

  override ready() {
    super.ready();
    this.initializeLoginScreen('EulaScreen');
  }

  private initializeScreen_() {
    if (this.initialized_) {
      return;
    }
    // this.$.eulaDialog.scrollToBottom();
    this.applyOobeConfiguration_();
    this.initialized_ = true;
  }

  private onPrivacyFrameLoad_() {
    // this.$.privacyDialog.scrollToBottom();
  }

  private applyOobeConfiguration_() {
    var configuration = Oobe.getInstance().getOobeConfiguration();
    if (!configuration) {
      return;
    }
    if (configuration.eulaAutoAccept) {
      this.eulaAccepted_();
    }
  }

  onEulaLoad_() {
    const eulaLoader = WebViewLoader.instances['crosEulaFrame'];
    const privacyLoader = WebViewLoader.instances['fydeosPrivacyFrame'];

    // if (eulaLoader.isPerformingRequests) {
    //   return;
    // }

    if (this.uiStep !== EulaScreenState.PRIVACY) {
      this.setUIStep(EulaScreenState.EULA);
    }
    // this.$.eulaDialog.scrollToBottom();
  }

  private loadEulaToWebview_(webview: chrome.webviewTag.WebView, onlineEulaUrl: string, clear_anchors: boolean) {
    var loadBundledEula = () => {
      WebViewHelper.loadUrlContentToWebView(
          webview, EULA_TERMS_URL, ContentType.HTML);
      this.offline_ = true;
    };

    this.offline_ = false;
    var eulaLoader = new WebViewLoader(
        webview, ONLINE_EULA_LOAD_TIMEOUT_IN_MS, loadBundledEula,
        clear_anchors, true);
    eulaLoader.setUrl(onlineEulaUrl);
  }

  override updateLocalizedContent() {
    this.i18nUpdateLocale();
  }

  private loadEula() {
    this.setUIStep(EulaScreenState.LOADING);

    const onlineEulaUrl = loadTimeData.getString('eulaOnlineUrl');
    const eulaFrame = this.shadowRoot?.querySelector<chrome.webviewTag.WebView>('#crosEulaFrame');
    assert(eulaFrame);
    this.loadEulaToWebview_(eulaFrame, onlineEulaUrl, true);

    const onlinePrivacyUrl = loadTimeData.getString('privacyOnlineUrl');
    const privacyFrame = this.shadowRoot?.querySelector<chrome.webviewTag.WebView>('#fydeosPrivacyFrame');
    assert(privacyFrame);
    this.loadEulaToWebview_(privacyFrame, onlinePrivacyUrl, true);
  }

  private eulaAccepted_() {
    this.userActed('accept-button');
  }

  private eulaFirstStepAccepted_() {
    if (this.offline_) {
      this.eulaAccepted_();
      return;
    }
    this.setUIStep(EulaScreenState.PRIVACY);
  }

  privacyAccepted_() {
    this.eulaAccepted_();
  }

  onPrivacyBackButtonPressed_() {
    this.setUIStep(EulaScreenState.EULA);
  }

  private onEulaBackButtonPressed_() {
    this.userActed('back-button');
  }
}

customElements.define(EulaScreen.is, EulaScreen);
