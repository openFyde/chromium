// os-settings-fydeos-experiment

import {PolymerElement} from 'chrome://resources/polymer/v3_0/polymer/polymer_bundled.min.js';
import {WebUiListenerMixin} from 'chrome://resources/cr_elements/web_ui_listener_mixin.js';
import {I18nMixin} from 'chrome://resources/cr_elements/i18n_mixin.js';
import {LifetimeBrowserProxyImpl} from '/shared/settings/lifetime_browser_proxy.js';
import {sendWithPromise} from 'chrome://resources/js/cr.js';
import {loadTimeData} from 'chrome://resources/js/load_time_data.js';
import '../../settings_shared.css.js';
import '../os_settings_page/os_settings_section.js';
import 'chrome://resources/cr_elements/cr_shared_style.css.js';
import 'chrome://resources/cr_elements/cr_toggle/cr_toggle.js';
import {getTemplate} from './fydeos_experiment.html.js';

const FydeSettingsExperimentPageElementBase =
    WebUiListenerMixin(I18nMixin(PolymerElement));

type TpmFallbackResponse = {
  current: boolean,
  pref: boolean,
}
/** @polymer */
class FydeSettingsExperimentPageElement extends FydeSettingsExperimentPageElementBase {
  static get is() {
    return 'os-settings-fydeos-experiment';
  }

  static get template() {
    return getTemplate();
  }

  static get properties() {
    return {
      isOwner_: {
        type: Boolean,
        value: false,
      },
      forceTpmFallback_: {
        type: Boolean,
        value: false,
      },

      currentForceTpmFallback_: {
        type: Boolean,
        value: false,
      },

      shouldShowTpmFallbackRestartButton_: {
        type: Boolean,
        computed: 'computeShouldShowRestartButton(forceTpmFallback_, currentForceTpmFallback_)',
      },

      isTpmFallbackNecessary_: {
        type: Boolean,
        value: () => {
          return loadTimeData.getBoolean('isTpmFallbackNecessary');
        },
      },

      shouldShowExperimentFeature_: {
        type: Boolean,
        computed: 'computeShouldShowExperimentFeature(shouldShowTpmFallback_)',
      },

      shouldShowTpmFallback_: {
        type: Boolean,
        computed: 'computeShouldShowTpmFallback(isTpmFallbackNecessary_, isOwner_)',
      },
      fydeExperimentTpmfallbackUrl_: {
        type: String,
        value: loadTimeData.getString('fydeExperimentTpmfallbackUrl'),
      }
    };
  }

  private isOwner_: boolean;
  private forceTpmFallback_: boolean;
  private currentForceTpmFallback_: boolean;
  private shouldShowTpmFallbackRestartButton_: boolean;
  private isTpmFallbackNecessary_: boolean;
  private shouldShowExperimentFeature_: boolean;
  private shouldShowTpmFallback_: boolean;

  constructor() {
    super();
    chrome.usersPrivate.getCurrentUser().then(
        (user: chrome.usersPrivate.User) => {
          this.isOwner_ = user.isOwner;
        });
  }

  override connectedCallback() {
    super.connectedCallback();
    this.getIsForceTpmFallback();
    this.addWebUiListener('force-tpm-fallback-changed', this.onForceTpmFallbackChanged_.bind(this));
  }

  getIsForceTpmFallback() {
    sendWithPromise('getIsForceTpmFallback').then((response) => {
      console.log('getIsForceTpmFallback', response);
      this.setForceTpmFallback(response);
    });
  }

  onForceTpmFallbackChanged_(response: TpmFallbackResponse) {
    console.log('onForceTpmFallbackChanged_', response);
    this.setForceTpmFallback(response);
  }

  onToggleForceTpmFallback_() {
    this.forceTpmFallback_ = !this.forceTpmFallback_;
    chrome.send('setForceTpmFallback', [this.forceTpmFallback_]);
  }

  setForceTpmFallback(response: TpmFallbackResponse) {
    const { current, pref } = response;
    this.forceTpmFallback_ = pref;
    this.currentForceTpmFallback_ = current;
  }

  onRestartTap_() {
    LifetimeBrowserProxyImpl.getInstance().signOutAndRestart();
  }

  computeShouldShowRestartButton(forceTpmFallback: boolean, currentForceTpmFallback: boolean) {
    return forceTpmFallback !== currentForceTpmFallback;
  }

  computeShouldShowTpmFallback(isNecessary: boolean, isOwner: boolean) {
    return isNecessary && isOwner;
  }

  computeShouldShowExperimentFeature(shouldShowTpmFallbackNecessary: boolean) {
    return shouldShowTpmFallbackNecessary;
  }

}

customElements.define(
    FydeSettingsExperimentPageElement.is, FydeSettingsExperimentPageElement);
