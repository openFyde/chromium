// os-settings-fydeos-tweak-ui
import {PolymerElement, mixinBehaviors} from 'chrome://resources/polymer/v3_0/polymer/polymer_bundled.min.js';
import {sendWithPromise} from 'chrome://resources/js/cr.js';
import {I18nMixin} from 'chrome://resources/ash/common/cr_elements/i18n_mixin.js';
import {WebUiListenerMixin} from 'chrome://resources/ash/common/cr_elements/web_ui_listener_mixin.js';
import 'chrome://resources/ash/common/cr_elements/cr_toggle/cr_toggle.js';
import 'chrome://resources/ash/common/cr_elements/cr_shared_style.css.js';
import '../settings_shared.css.js';

import {getTemplate} from './fydeos_tweak_ui.html.js';

const FydeSettingsTweakUIPageElementBase =
    WebUiListenerMixin(I18nMixin(PolymerElement));

class FydeSettingsTweakUiPageElement extends FydeSettingsTweakUIPageElementBase {
  static get is() {
    return 'os-settings-fydeos-tweak-ui' as const;
  }

  static get template() {
    return getTemplate();
  }

  static get properties() {
    return {
      showRotateScreenButton_: Boolean,
      isInTabletPhysicalState_: Boolean,
      showSwitchTabletLaptopButton_: Boolean,
      canToggleRotateScreenButton_: {
        type: Boolean,
        computed: 'computeCanToggleRotateScreenButton_(isInTabletPhysicalState_)',
      },
    };
  }

  private showRotateScreenButton_: boolean;
  private isInTabletPhysicalState_: boolean;
  private showSwitchTabletLaptopButton_: boolean;
  private canToggleRotateScreenButton_: boolean;

  override connectedCallback() {
    super.connectedCallback();
    this.getShowRotateScreenButton();
    this.getIsInTabletPhysicalState();
    this.getShowSwitchTabletLaptopButton();
    this.addWebUiListener('show-rotate-screen-button-changed', this.onShowRotateScreenButtonChanged_.bind(this));
    this.addWebUiListener('is-in-tablet-physical-state-changed', this.onIsInTabletPhysicalStateChanged_.bind(this));
    this.addWebUiListener('show-switch-tablet-laptop-button-changed', this.onShowSwitchTabletLaptopButtonChanged_.bind(this));
  }

  getShowRotateScreenButton() {
    sendWithPromise('getShowRotateScreenButton').then((visible) => {
      this.showRotateScreenButton_ = visible;
    });
  }

  getIsInTabletPhysicalState() {
    sendWithPromise('getIsInTabletPhysicalState').then((enabled) => {
      this.isInTabletPhysicalState_ = enabled;
    });
  }

  onShowRotateScreenButtonChanged_(visible: boolean) {
    this.showRotateScreenButton_ = visible;
  }

  onIsInTabletPhysicalStateChanged_(enabled: boolean) {
    this.isInTabletPhysicalState_ = enabled;
  }

  computeCanToggleRotateScreenButton_(isInTabletPhysicalState: boolean) {
    const canToggleRotateScreenButton = isInTabletPhysicalState;
    return canToggleRotateScreenButton;
  }

  onToggleShowRotateScreenButton_() {
    this.showRotateScreenButton_ = !this.showRotateScreenButton_;
    chrome.send('setShowRotateScreenButton', [this.showRotateScreenButton_]);
  }

  shouldShowRotateScreenButton_() {
    return this.showRotateScreenButton_ && this.canToggleRotateScreenButton_;
  }

  getShowRotateScreenButtonMessage_() {
    if (!this.isInTabletPhysicalState_) {
      return this.i18nAdvanced('notTabletPhysicalStateDisableFydeOsRotateScreen');
    }
    return this.i18nAdvanced('displayFydeOsRotateScreenButton');
  }

  getShowSwitchTabletLaptopButton() {
    sendWithPromise('getShowSwitchTabletLaptopButton').then((enabled) => {
      console.log('getShowSwitchTabletLaptopButton', enabled);
      this.showSwitchTabletLaptopButton_ = enabled;
    });
  }

  onToggleShowSwitchTabletLaptopButton_() {
    this.showSwitchTabletLaptopButton_ = !this.showSwitchTabletLaptopButton_;
    chrome.send('setShowSwitchTabletLaptopButton', [this.showSwitchTabletLaptopButton_]);
  }

  onShowSwitchTabletLaptopButtonChanged_(visible: boolean) {
    console.log('onShowSwitchTabletLaptopButtonChanged_', visible);
    this.showSwitchTabletLaptopButton_ = visible;
  }
}

customElements.define(
    FydeSettingsTweakUiPageElement.is, FydeSettingsTweakUiPageElement);
