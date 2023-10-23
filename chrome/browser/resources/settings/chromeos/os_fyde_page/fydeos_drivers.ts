// os-settings-fydeos-drivers

import {PolymerElement} from 'chrome://resources/polymer/v3_0/polymer/polymer_bundled.min.js';
import 'chrome://resources/polymer/v3_0/iron-flex-layout/iron-flex-layout-classes.js';
import 'chrome://resources/polymer/v3_0/paper-tooltip/paper-tooltip.js'
import 'chrome://resources/cr_elements/md_select.css.js';
import {I18nMixin} from 'chrome://resources/cr_elements/i18n_mixin.js';
import 'chrome://resources/cr_elements/cr_shared_style.css.js';

import {Route} from '../router.js';
import {routes} from '../os_settings_routes.js';
import {RouteObserverMixin} from '../route_observer_mixin.js'
import '../../settings_shared.css.js';
import '../os_settings_page/os_settings_section.js';
import {ShellClient} from './shell_client.js';

import {getTemplate} from './fydeos_drivers.html.js';

class DriverClient {
  private element_: HTMLElement;
  private shellClient_: ShellClient;

  constructor(ele: HTMLElement) {
    this.element_ = ele;
    this.shellClient_ = new ShellClient(ele);
  }

  static get TouchpadDrivers() {
    return [ 'bare', 'imps', 'exps', 'auto' ];
  }

  static get WifiDrivers() {
    return [ 'wl', 'brcm' ];
  }

  static get Events() {
    return {
      TOUCHPAD_DRIVER_CHANGED: 'touchpadDriverChanged',
      WIFI_DRIVER_CHANGED: 'wifiDriverChanged',
    };
  }

  static get wifiDriverCommand() {
    return '/usr/bin/switch-brcm-driver';
  }

  static get touchpadCommand() {
    return '/usr/bin/set-touchpad.sh';
  }

  static get wifiDriverConfigFile() {
    return '/etc/modprobe.d/flintos-brcm.conf';
  }

  static get touchpadConfigFile() {
    return '/etc/modprobe.d/fydeos-psmouse.conf';
  }

  async IsTweakWifiSupported() {
    const ret = await this.shellClient_.IsFileExist(DriverClient.wifiDriverCommand);
    return ret;
  }

  async IsTweakTouchpadSupported() {
    const ret = await this.shellClient_.IsFileExist(DriverClient.touchpadCommand);
    return ret;
  }

  async getTouchpadDriver() {
    let mode = '';
    let content = await this.shellClient_.SafeExecForResult(`cat ${DriverClient.touchpadConfigFile}`);
    if (!content) {
      mode = 'auto';
    } else {
      const a = content.split(/\r?\n/);
      if (a.length) {
        const text = a[0];
        const b = text.split('=');
        if (b.length >= 2) {
          mode = b[1].trim();
        } else {
          mode = 'auto';
        }
      } else {
        mode = 'auto';
      }
    }
    return mode;
  }

  async setTouchpadDriver(driver: string) {
    await this.shellClient_.SafeExecForResult(`${DriverClient.touchpadCommand} ${driver}`);
    this.updateTouchpadDriver();
  }

  async updateTouchpadDriver() {
    const driver = await this.getTouchpadDriver();
    const options = DriverClient.TouchpadDrivers.map(n => ({
      name: n, value: n, selected: n === driver,
    }));
    this.shellClient_.dispatchEvent(DriverClient.Events.TOUCHPAD_DRIVER_CHANGED, { detail: { options } });
  }

  async getWifiDriver() {
    const content = await this.shellClient_.SafeExecForResult(`cat ${DriverClient.wifiDriverConfigFile}`);
    const wlDisabled = (content.indexOf('blacklist wl') !== -1);
    return wlDisabled ? 'brcm' : 'wl';
  }

  async setWifiDriver(driver: string) {
    await this.shellClient_.SafeExecForResult(`${DriverClient.wifiDriverCommand} ${driver}`);
    this.updateWifiDriver();
  }

  async updateWifiDriver() {
    const driver = await this.getWifiDriver();
    const options = DriverClient.WifiDrivers.map(n => ({
      name: n, value: n, selected: n === driver,
    }));
    this.shellClient_.dispatchEvent(DriverClient.Events.WIFI_DRIVER_CHANGED, { detail: { options } });
  }

  Restart() {
    return this.shellClient_.ExecForResult('powerd_setuid_helper --action=reboot');
  }
}

const FydeSettingsDriversPageElementBase = 
  RouteObserverMixin(I18nMixin(PolymerElement));

type DriverOption = {
  name: string,
  value: string,
  selected: boolean,
};
/** @polymer */
class FydeSettingsDriversPageElement extends FydeSettingsDriversPageElementBase {
  static get is() {
    return 'os-settings-fydeos-drivers' as const;
  }

  static get template() {
    return getTemplate();
  }

  static get properties() {
    return {
      tweakTouchpadSupported_: {
        type: Boolean,
        value: true,
      },
      tweakWifiSupported_: {
        type: Boolean,
        value: true,
      },
      touchpadDriverOptions_: {
        type: Array,
        value() {
          return [];
        },
      },
      wifiDriverOptions_: {
        type: Array,
        value() {
          return [];
        },
      },
      initWifiDriver_: {
        type: String,
        value: '',
      },
      initTouchpadDriver_: {
        type: String,
        value: '',
      },
    };
  }

  private driverClient_: DriverClient;
  private tweakWifiSupported_: boolean;
  private tweakTouchpadSupported_: boolean;
  private touchpadDriverOptions_: Array<DriverOption>;
  private wifiDriverOptions_: Array<DriverOption>;
  private initTouchpadDriver_: string;
  private initWifiDriver_: string;

  constructor() {
    super();
    console.log('os-settings-fydeos-drivers ready');
    this.driverClient_ =  new DriverClient(this);
    this.checkIsSupported_();
    this.addListeners();
  }

  override connectedCallback() {
    super.connectedCallback();
    console.log('os-settings-fydeos-drivers attached');
    this.updateDrivers_();
  }

  async checkIsSupported_() {
    this.tweakWifiSupported_ = await this.driverClient_.IsTweakWifiSupported();
    this.tweakTouchpadSupported_ = await this.driverClient_.IsTweakTouchpadSupported();
  }

  override currentRouteChanged(currentRoute: Route) {
    if (currentRoute !== routes.FYDEOS) {
      return;
    }
    this.updateDrivers_();
  }

  addListeners() {
    this.addEventListener(DriverClient.Events.TOUCHPAD_DRIVER_CHANGED, this.onTouchpadDriverChanged_.bind(this));
    this.addEventListener(DriverClient.Events.WIFI_DRIVER_CHANGED, this.onWifiDriverChanged_.bind(this));
  }

  updateDrivers_() {
    this.updateTouchpadDriver_();
    this.updateWifiDriver_();
  }

  updateTouchpadDriver_() {
    if (this.tweakTouchpadSupported_ && this.driverClient_) {
      this.driverClient_.updateTouchpadDriver();
    }
  }

  updateWifiDriver_() {
    if (this.tweakWifiSupported_ && this.driverClient_) {
      this.driverClient_.updateWifiDriver();
    }
  }

  onTouchpadSelectChange_() {
    const ele = this.shadowRoot!.querySelector('#touchpadDriverSelect') as HTMLInputElement;
    if (!ele) return;
    const driver = ele.value;
    this.driverClient_.setTouchpadDriver(driver);
  }

  onTouchpadDriverChanged_(e: Event) {
    console.log('onTouchpadDriverChanged_', e);
    const { detail: { options } } = e as CustomEvent;
    this.touchpadDriverOptions_ = options;
  }

  onWifiSelectChange_() {
    const ele = this.shadowRoot!.querySelector('#wifiDriverSelect') as HTMLInputElement;
    if (!ele) return;
    const driver = ele.value;
    this.driverClient_.setWifiDriver(driver);
  }

  onWifiDriverChanged_(e: Event) {
    console.log('onWifiDriverChanged_', e);
    const { detail: { options } } = e as CustomEvent;
    this.wifiDriverOptions_ = options;
    this.setInitWifiDriver_(options);
  }

  setInitWifiDriver_(options: Array<DriverOption>) {
    if (!this.initWifiDriver_) {
      const selected = options.find(n => n.selected);
      if (selected) {
        this.initWifiDriver_ = selected.value;
      }
    }
  }

  shouldShowRestartButton_() {
    if (!this.initWifiDriver_) return false;
    const current = this.wifiDriverOptions_.find(n => n.selected);
    if (!current) return false;
    return current.value !== this.initWifiDriver_;
  }

  async onRestartTap_() {
    await this.driverClient_.Restart();
  }
}

customElements.define(
    FydeSettingsDriversPageElement.is, FydeSettingsDriversPageElement);
