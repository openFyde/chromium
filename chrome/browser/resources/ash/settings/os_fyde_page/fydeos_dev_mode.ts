import {PolymerElement} from 'chrome://resources/polymer/v3_0/polymer/polymer_bundled.min.js';
import {WebUiListenerMixin} from 'chrome://resources/ash/common/cr_elements/web_ui_listener_mixin.js';
import {I18nMixin} from 'chrome://resources/ash/common/cr_elements/i18n_mixin.js';
import {LifetimeBrowserProxyImpl} from '/shared/settings/lifetime_browser_proxy.js';
import {sendWithPromise} from 'chrome://resources/js/cr.js';
import {getTemplate} from './fydeos_dev_mode.html.js';
import {loadTimeData} from 'chrome://resources/js/load_time_data.js';

const FydeSettingsDevModePageElementBase =
    WebUiListenerMixin(I18nMixin(PolymerElement));

/** @polymer */
class FydeSettingsDevModePageElement extends FydeSettingsDevModePageElementBase {
  static get is() {
    return 'os-settings-fydeos-dev-mode';
  }
  static get template() {
    return getTemplate();
  }
  static get properties() {
    return {
      currentDevMode_: {
        type: Boolean,
      },
      switchSupported_: {
        type: Boolean,
        value: true,
      },
      isGuest_: {
        type: Boolean,
        value: () => {
          return loadTimeData.getBoolean('isGuest');
        },
      },
      showDevModeTransition_: {
        type: Boolean,
        computed: 'computeShouldShowModeTransition_(switchSupported_, isGuest_)',
      },
      buttonDisabled_: {
        type: Boolean,
        computed: 'computeShouldDisableTransitButton_(currentDevMode_, showConfirmDialog_)',
      },
      showConfirmDialog_: {
        type: Boolean,
        value: false,
      },
      showTransitButton_: {
        type: Boolean,
        computed: 'computeShouldShowTransitButton_(currentDevMode_, showRestartButton_)',
      },
      showRestartButton_: {
        type: Boolean,
        value: false,
      },
    };
  }

  private showDevModeTransition_: boolean;
  private buttonDisabled_: boolean;
  private showConfirmDialog_: boolean;
  private readonly currentDevMode_: boolean;
  private switchSupported_: boolean;
  private showRestartButton_: boolean;
  private showTransitButton_: boolean;
  private isGuest_: boolean;

  constructor() {
    super();
    this.currentDevMode_ = loadTimeData.valueExists('devMode') && loadTimeData.getBoolean('devMode');
    this.showTransitButton_ = !this.currentDevMode_;
  }

  override connectedCallback() {
    super.connectedCallback();
    if (!this.isGuest_) {
      this.checkDevModeSwitchSupported_();
    }
  }

  checkDevModeSwitchSupported_() {
    sendWithPromise('getDevModeSwitchSupported').then((result: boolean) => {
      this.switchSupported_ = result;
    });
  }

  computeShouldShowModeTransition_(switchSupported: boolean, isGuest: boolean): boolean {
    return switchSupported && !isGuest;
  }

  computeShouldDisableTransitButton_(currentDevMode: boolean, showConfirmDialog: boolean): boolean {
    return currentDevMode || showConfirmDialog;
  }

  computeShouldShowTransitButton_(_currentDevMode: boolean, showRestartButton: boolean): boolean {
    // return !_currentDevMode && !showRestartButton;
    return !showRestartButton; // just disable the button if current is already dev mode, see computeShouldDisableTransitButton_
  }

  onButtonClick_() {
    this.showConfirmDialog_ = true;
  }

  onDialogCancel_() {
    this.showConfirmDialog_ = false;
  }

  onDialogConfirm_() {
    this.showConfirmDialog_ = false;
    sendWithPromise('setDevMode', !this.currentDevMode_).then((result: boolean) => {
      if (!result) {
        console.log('Failed to set dev mode');
      } else {
        this.showRestartButton_ = true;
      }
    });
  }

  onRestart_() {
    LifetimeBrowserProxyImpl.getInstance().relaunch();
  }
}

customElements.define(
    FydeSettingsDevModePageElement.is, FydeSettingsDevModePageElement);
