// os-settings-fyde-page

import {PolymerElement} from 'chrome://resources/polymer/v3_0/polymer/polymer_bundled.min.js';
import 'chrome://resources/cr_elements/cr_shared_style.css.js';

import {I18nMixin} from 'chrome://resources/cr_elements/i18n_mixin.js';
import {RouteObserverMixin} from '../route_observer_mixin.js'
import {Route} from '../router.js';
import {loadTimeData} from 'chrome://resources/js/load_time_data.js';
import '../os_settings_page/os_settings_section.js';
import '../os_settings_page/os_settings_subpage.js';
import '../os_settings_page_styles.css.js';
import '../../settings_shared.css.js';
import './fydeos_account.js';
import './fydeos_drivers.js';
import './fydeos_remoting.js';
import './fydeos_tweak_ui.js';
import './fydeos_experiment.js';
import './fydeos_more_info.js';

import {getTemplate} from './os_fyde_page.html.js';

const OsSettingsFydePageElementBase = 
  RouteObserverMixin(I18nMixin(PolymerElement));

/** @polymer */
class OsSettingsFydePageElement extends OsSettingsFydePageElementBase {
  static get is() {
    return 'os-settings-fyde-page' as const;
  }

  static get template() {
    return getTemplate();
  }

  static get properties() {
    return {
      isFydeAccount_: {
        type: Boolean,
        value() {
          return loadTimeData.getBoolean('isFydeProfile');
        },
      },

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
    };
  }

  private isFydeAccount_: boolean;
  private showToggleRebootButtonInTray: boolean;
  private showToggleRotateScreenButton: boolean;

  override currentRouteChanged(newRoute: Route, preRoute: Route) {
    console.log('currentRouteChanged_', preRoute, newRoute);
  }

  constructor() {
    super();
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
    return this.showToggleRotateScreenButton || this.showToggleRebootButtonInTray;
  }
}

customElements.define(
    OsSettingsFydePageElement.is, OsSettingsFydePageElement);
