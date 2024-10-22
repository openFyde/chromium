import 'chrome://resources/ash/common/cr_elements/cr_link_row/cr_link_row.js';
import '../controls/settings_toggle_button.js';
import '../os_settings_page/settings_card.js';
import '../settings_shared.css.js';

import {PrefsMixin} from '/shared/settings/prefs/prefs_mixin.js';
import {loadTimeData} from 'chrome://resources/js/load_time_data.js';
import {PolymerElement} from 'chrome://resources/polymer/v3_0/polymer/polymer_bundled.min.js';
import {Section} from '../mojom-webui/routes.mojom-webui.js';

import {getTemplate} from './fyde_assistant_page.html.js';

const OsSettingsFydeAssistantPageElementBase = PrefsMixin(PolymerElement);

export class OsSettingsFydeAssistantPageElement extends OsSettingsFydeAssistantPageElementBase {
  static get is() {
    return 'os-settings-fyde-assistant-page';
  }

  static get template() {
    return getTemplate();
  }

  static get properties() {
    return {
      section_: {
        type: Number,
        value: Section.kFydeAssistant,
        readOnly: true,
      },
    };
  }

  // constructor() {
  //   super();
  // }
  //
  // override ready() {
  //   super.ready();
  // }
  //
  // override connectedCallback() {
  //   super.connectedCallback();
  // }
  onSettingsClicked_() {
    let enabled = false;
    try {
       enabled = this.getPref<boolean>('fyde_assistant_enabled').value;
    } catch (e) {
      console.error('Get pref fyde_assistant_enabled error', e);
    }

    if (!enabled) return;
    window.open('chrome://fydeos-ai/settings');
  }
}

declare global {
  interface HTMLElementTagNameMap {
    'os-settings-fyde-assistant-page': OsSettingsFydeAssistantPageElement;
  }
}

customElements.define(OsSettingsFydeAssistantPageElement.is, OsSettingsFydeAssistantPageElement);
