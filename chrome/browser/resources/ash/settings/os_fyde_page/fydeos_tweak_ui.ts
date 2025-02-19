// os-settings-fydeos-tweak-ui
import {PolymerElement, mixinBehaviors} from 'chrome://resources/polymer/v3_0/polymer/polymer_bundled.min.js';
import {sendWithPromise} from 'chrome://resources/js/cr.js';
import {I18nMixin} from 'chrome://resources/ash/common/cr_elements/i18n_mixin.js';
import {WebUiListenerMixin} from 'chrome://resources/ash/common/cr_elements/web_ui_listener_mixin.js';
import {CrDialogElement} from 'chrome://resources/ash/common/cr_elements/cr_dialog/cr_dialog.js';
import {LifetimeBrowserProxyImpl} from '/shared/settings/lifetime_browser_proxy.js';
import 'chrome://resources/ash/common/cr_elements/cr_toggle/cr_toggle.js';
import 'chrome://resources/ash/common/cr_elements/cr_dialog/cr_dialog.js';
import 'chrome://resources/ash/common/cr_elements/cr_shared_style.css.js';
import '../settings_shared.css.js';
import {ShellClient} from './shell_client.js';

import {getTemplate} from './fydeos_tweak_ui.html.js';

class WidevineHelper {
  private element_: HTMLElement;
  private shellClient_: ShellClient;

  constructor(ele: HTMLElement) {
    this.element_ = ele;
    this.shellClient_ = new ShellClient(ele);
  }

  static get Command() {
    return '/usr/bin/enable_libwidevine';
  }

  static get RunningStateParam() {
    return '--state';
  }

  static get StatusParam() {
    return '--status';
  }

  static get EnableParam() {
    return '--file';
  }

  static get DisableParam() {
    return '--disable';
  }

  async isSupported() {
    const ret = await this.shellClient_.IsFileExist(WidevineHelper.Command);
    return ret;
  }

  async getRunningState() {
    const result = await this.shellClient_.SafeExecForResult(`${WidevineHelper.Command} ${WidevineHelper.RunningStateParam}`);
    return result;
  }

  async getWidevineStatus() {
    const result = await this.shellClient_.SafeExecForResult(`${WidevineHelper.Command} ${WidevineHelper.StatusParam}`);
    return result;
  }

  async toggle(file: string | null) {
    if (file) {
      await this.enable(file);
    } else {
      await this.disable();
    }
  }

  async install(file: string) {
    await this.shellClient_.ExecForResult(
      `${WidevineHelper.Command} ${WidevineHelper.EnableParam} ${file}`);
  }

  async enable(file: string) {
    try {
      await this.install(file);
    } catch (err) {
      console.log('enable widevine failed', err);
      throw err;
    }
  }

  async disable() {
    await this.shellClient_.SafeExecForResult(`${WidevineHelper.Command} ${WidevineHelper.DisableParam}`);
  }
}

const WIDEVINE_STATUS_KEY = 'fydeos_libwidevine_enabled';

interface FydeSettingsTweakUiPageElement {
  $: {
    widevineErrorDialog: CrDialogElement,
  };
}

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
      showToggleWidevine_: {
        type: Boolean,
        value: false,
      },
      libwidevineEnabled_: {
        type: Boolean,
        value: false,
      },
      togglingWidevine_: {
        type: Boolean,
        value: false,
      },
      showWidevineErrorDialog_: {
        type: Boolean,
        value: false,
      },
      rebootRequiredForWidevine_: {
        type: Boolean,
        value: false,
      }
    };
  }

  private showRotateScreenButton_: boolean;
  private isInTabletPhysicalState_: boolean;
  private showSwitchTabletLaptopButton_: boolean;
  private canToggleRotateScreenButton_: boolean;

  private showToggleWidevine_: boolean;
  private libwidevineEnabled_: boolean;
  private togglingWidevine_: boolean;

  private client_: WidevineHelper;

  private showWidevineErrorDialog_: boolean;
  private rebootRequiredForWidevine_: boolean;

  constructor() {
    super();
    this.client_ =  new WidevineHelper(this);
  }

  override connectedCallback() {
    super.connectedCallback();
    this.getShowRotateScreenButton();
    this.getIsInTabletPhysicalState();
    this.getShowSwitchTabletLaptopButton();
    this.addWebUiListener('show-rotate-screen-button-changed', this.onShowRotateScreenButtonChanged_.bind(this));
    this.addWebUiListener('is-in-tablet-physical-state-changed', this.onIsInTabletPhysicalStateChanged_.bind(this));
    this.addWebUiListener('show-switch-tablet-laptop-button-changed', this.onShowSwitchTabletLaptopButtonChanged_.bind(this));
    this.addWebUiListener('fydeos-libwidevine-file-selected', this.onLibwidevineFileSelected_.bind(this));

    this.checkLibwidevineStatus_();
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

  async checkLibwidevineStatus_() {
    const support = await this.client_.isSupported();
    if (support) {
      this.getLibwidevineScriptRunningState_();
      const result = await this.getLibwidevineEnabled_();
      // if we can't get 'yes' or 'no' throught the script, do not show libwidevine settings block
      this.showToggleWidevine_ = !!result;
    }
    if (this.showToggleWidevine_) {
      this.getRebootRequiredForWidevine_();
    }
  }

  async getLibwidevineEnabled_() {
    const result = await this.client_.getWidevineStatus();
    const enabled = (result === 'yes');
    this.libwidevineEnabled_ = enabled;
    return result;
  }

  async getLibwidevineScriptRunningState_() {
    const result = await this.client_.getRunningState();
    this.togglingWidevine_ = (result === 'yes');
  }

  async onToggleLibwidevine_() {
    if (this.togglingWidevine_) {
      return;
    }
    const enabled = !this.libwidevineEnabled_;
    this.libwidevineEnabled_ = enabled;
    if (enabled) {
      chrome.send('selectLibwidevineFile', []);
    } else {
      await this.disableLibwidevine_();
    }
  }

  async enableLibwidevine_(file: string) {
    this.togglingWidevine_ = true;
    try {
      await this.client_.toggle(file);
    } catch (e) {
      this.showWidevineErrorDialog_ = true;
    }
    this.togglingWidevine_ = false;
    const result = await this.getLibwidevineEnabled_();
    if (result === 'yes') {
      this.toggleRebootRequiredForWidevine_(true);
    }
  }

  async disableLibwidevine_() {
    this.togglingWidevine_ = true;
    await this.client_.toggle(null);
    this.togglingWidevine_ = false;
    const result = await this.getLibwidevineEnabled_();
    if (result === 'no') {
      this.toggleRebootRequiredForWidevine_(false);
    }
  }

  getRebootRequiredForWidevine_() {
    sendWithPromise('getRebootRequiredForWidevine').then((required: boolean) => {
      this.rebootRequiredForWidevine_ = required;
    });
  }

  toggleRebootRequiredForWidevine_(force: boolean) {
    sendWithPromise('toggleRebootRequiredForWidevine', force).then((required: boolean) => {
      this.rebootRequiredForWidevine_ = required;
    });
  }

  shouldShowRebootButtonForWidevine_(toggling: boolean, required: boolean) {
    return !toggling && required;
  }

  onRestartForWidevineTap_() {
    LifetimeBrowserProxyImpl.getInstance().signOutAndRestart();
  }

  toggleWidevineHelpMsg_() {
    return this.i18nAdvanced('toggleWidevineHelpMessage');
  }

  onWidevineErrorDialogClose_() {
    this.showWidevineErrorDialog_ = false;
  }

  async onLibwidevineFileSelected_(file: string | null) {
    if (file) {
      await this.enableLibwidevine_(file);
    } else {
      this.libwidevineEnabled_ = false;
    }
  }
}

customElements.define(
    FydeSettingsTweakUiPageElement.is, FydeSettingsTweakUiPageElement);
