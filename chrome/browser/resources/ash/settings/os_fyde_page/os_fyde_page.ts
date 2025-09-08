// os-settings-fyde-page

import {PolymerElement} from 'chrome://resources/polymer/v3_0/polymer/polymer_bundled.min.js';
import 'chrome://resources/ash/common/cr_elements/cr_shared_style.css.js';

import {I18nMixin} from 'chrome://resources/ash/common/cr_elements/i18n_mixin.js';
import {Router, routes} from '../router.js';
import {RouteOriginMixin} from '../common/route_origin_mixin.js';
import {Section} from '../mojom-webui/routes.mojom-webui.js';
import {loadTimeData} from 'chrome://resources/js/load_time_data.js';
import '../os_settings_page/settings_card.js';
import '../os_settings_page/os_settings_subpage.js';
import '../settings_shared.css.js';
import './fydeos_account.js';
// import './fydeos_drivers.js';
import './fydeos_remoting.js';
import './fydeos_tweak_ui.js';
import './fydeos_experiment.js';
import './fydeos_more_info.js';
import './fydeos_dev_mode.js';
// <if expr="use_fydeos_license">
import './fydeos_license_info.js';
// </if>

import {getTemplate} from './os_fyde_page.html.js';

const OsSettingsFydePageElementBase = 
  RouteOriginMixin(I18nMixin(PolymerElement));

export class OsSettingsFydePageElement extends OsSettingsFydePageElementBase {
  static get is() {
    return 'os-fyde-page' as const;
  }

  static get template() {
    return getTemplate();
  }

  static get properties() {
    return {
      section_: {
        type: Number,
        value: Section.kFydeOs,
        readOnly: true,
      },
      isFydeAccount_: {
        type: Boolean,
        value() {
          return loadTimeData.getBoolean('isFydeProfile');
        },
      },
      isGuest_: {
        type: Boolean,
        value() {
          return loadTimeData.getBoolean('isGuest');
        },
      },

// <if expr="use_fydeos_license">
      showFydeOsLicense_: {
        type: Boolean,
        value() {
          return loadTimeData.valueExists('showFydeOsLicense') && loadTimeData.getBoolean('showFydeOsLicense');
        },
      },
// </if>

      showToggleRebootButtonInTray: {
        type: Boolean,
        value() {
          // some specified board cannot reboot
          const ret = loadTimeData.getBoolean('showToggleRebootButtonInTray');
          console.log('showToggleRebootButtonInTray', ret);
          return ret;
        },
      },
      showToggleRotateScreenButton: {
        type: Boolean,
        value() {
          // only amd64-fydeos/amd64-generic shouldShowToggleShowRotateScreenButton is true
          return loadTimeData.getBoolean('showToggleRotateScreenButton');
        },
      },
      showToggleSwitchTabletLaptopButton: {
        type: Boolean,
        value() {
          return loadTimeData.getBoolean('showToggleSwitchTabletLaptopButton');
        },
      },

      authFactorHasPassword_: Boolean,
    };
  }

  private section_: number;
  private isFydeAccount_: boolean;
  private isGuest_: boolean;
  private showFydeOsLicense_: boolean;
  private showToggleRebootButtonInTray: boolean;
  private showToggleRotateScreenButton: boolean;
  private showToggleSwitchTabletLaptopButton: boolean;
  private authFactorHasPassword_: boolean;

  constructor() {
    super();
    this.route = routes.FYDEOS;
    console.log('os-settings-fyde-page ready');
    // <if expr="_google_chrome">
    console.log('this is a test for build tools')
    // </if>
  }

  override connectedCallback() {
    super.connectedCallback();
    console.log('os-settings-fyde-page attached');
  }

  showFydeOsTweakUi_() {
    return this.showToggleRotateScreenButton || this.showToggleRebootButtonInTray || this.showToggleSwitchTabletLaptopButton;
  }

  onAuthFactorHasPasswordChanged_(e: CustomEvent) {
    this.authFactorHasPassword_ = e.detail;
  }

// <if expr="use_fydeos_license">
  onLicenseInfoClick_() {
    if (!this.showFydeOsLicense_) return;
    Router.getInstance().navigateTo(routes.FYDEOS_LICENSE_INFO);
  }
// </if>
}

customElements.define(
    OsSettingsFydePageElement.is, OsSettingsFydePageElement);
