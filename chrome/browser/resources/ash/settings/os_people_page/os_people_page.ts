// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview
 * 'settings-people-page' is the settings page containing sign-in settings.
 */
import 'chrome://resources/ash/common/cr_elements/cr_icon_button/cr_icon_button.js';
import 'chrome://resources/ash/common/cr_elements/cr_link_row/cr_link_row.js';
import 'chrome://resources/ash/common/cr_elements/icons.html.js';
import 'chrome://resources/ash/common/cr_elements/policy/cr_policy_indicator.js';
import 'chrome://resources/ash/common/cr_elements/cr_shared_vars.css.js';
import 'chrome://resources/polymer/v3_0/iron-flex-layout/iron-flex-layout-classes.js';
import '../controls/settings_toggle_button.js';
import '../settings_shared.css.js';
import '../os_settings_page/os_settings_animated_pages.js';
import '../os_settings_page/os_settings_subpage.js';
import '../os_settings_page/settings_card.js';
import '../parental_controls_page/parental_controls_settings_card.js';
import './account_manager_settings_card.js';
import './additional_accounts_settings_card.js';
import './graduation/graduation_settings_card.js';

import {WebUiListenerMixin} from 'chrome://resources/ash/common/cr_elements/web_ui_listener_mixin.js';
import {loadTimeData} from 'chrome://resources/js/load_time_data.js';
import {PolymerElement} from 'chrome://resources/polymer/v3_0/polymer/polymer_bundled.min.js';

import {isAccountManagerEnabled} from '../common/load_time_booleans.js';
import type {PrefsState} from '../common/types.js';
import {type GraduationHandlerInterface, GraduationObserverReceiver} from '../mojom-webui/graduation_handler.mojom-webui.js';
import {Section} from '../mojom-webui/routes.mojom-webui.js';

import {AccountManagerBrowserProxyImpl} from './account_manager_browser_proxy.js';
import type {Account} from './account_manager_browser_proxy.js';
import {getGraduationHandlerProvider} from './graduation/mojo_interface_provider.js';
import {getTemplate} from './os_people_page.html.js';

import 'chrome://resources/ash/common/cr_elements/localized_link/localized_link.js';
import {I18nMixin} from 'chrome://resources/ash/common/cr_elements/i18n_mixin.js';
import {getImage} from 'chrome://resources/js/icon.js';
import {convertImageSequenceToPng} from 'chrome://resources/ash/common/cr_picture/png.js';
import type {SyncStatus} from '/shared/settings/people_page/sync_browser_proxy.js';
import {SignedInState, SyncBrowserProxyImpl} from '/shared/settings/people_page/sync_browser_proxy.js';
import type {ProfileInfo} from '/shared/settings/people_page/profile_info_browser_proxy.js';
import {ProfileInfoBrowserProxyImpl} from '/shared/settings/people_page/profile_info_browser_proxy.js';
import {Router, routes} from '../router.js';

const OsSettingsPeoplePageElementBase = WebUiListenerMixin(I18nMixin(PolymerElement));

export class OsSettingsPeoplePageElement extends
    OsSettingsPeoplePageElementBase {
  static get is() {
    return 'os-settings-people-page' as const;
  }

  static get template() {
    return getTemplate();
  }

  static get properties() {
    return {
      prefs: {
        type: Object,
        notify: true,
      },

      section_: {
        type: Number,
        value: Section.kPeople,
        readOnly: true,
      },

      accounts_: {
        type: Array,
        value() {
          return [];
        },
      },

      deviceAccount_: {
        type: Object,
        value() {
          return null;
        },
      },

      isAccountManagerEnabled_: {
        type: Boolean,
        value() {
          return isAccountManagerEnabled();
        },
        readOnly: true,
      },

      isProfileActionable_: {
        type: Boolean,
        value: function() {
          if (loadTimeData.getBoolean('isFydeProfile')) {
            return !loadTimeData.getBoolean('isFydeLocalAccount');
          }
          return false;
        },
      },

      profileActionButtonIcon_: {
        type: String,
          value: function() {
            if (loadTimeData.getBoolean('isFydeProfile')) {
              return 'icon-external';
            }
            return 'subpage-arrow';
          },
      },

      isFydeProfile_: {
        value() {
          return loadTimeData.getBoolean('isFydeProfile');
        }
      },

      isFydeLocalAccount_: {
        type: Boolean,
        value() {
          return loadTimeData.getBoolean('isFydeLocalAccount');
        },
      },

      profileIconUrl_: String,

      profileName_: String,

      profileLabel_: String,

      showParentalControls_: {
        type: Boolean,
        value() {
          return loadTimeData.valueExists('showParentalControls') &&
              loadTimeData.getBoolean('showParentalControls') &&
             (!loadTimeData.valueExists('isFydeProfile') ||
              !loadTimeData.getBoolean('isFydeProfile'));
        },
      },

      showGraduationApp_: {
        type: Boolean,
        value: () => {
          return loadTimeData.getBoolean('isGraduationFlagEnabled') &&
              loadTimeData.getBoolean('isGraduationAppEnabled');
        },
      },
    };
  }

  prefs: PrefsState;
  private accounts_: Account[];
  private deviceAccount_: Account|null;
  private graduationMojoProvider_: GraduationHandlerInterface;
  private graduationObserverReceiver_: GraduationObserverReceiver|null;
  private isAccountManagerEnabled_: boolean;
  private showGraduationApp_: boolean;
  private showParentalControls_: boolean;
  private section_: Section;

  private isProfileActionable_: boolean;
  private profileActionButtonIcon_: string;
  private isFydeProfile_: boolean;
  private isFydeLocalAccount_: boolean;
  private profileIconUrl_: string;
  private profileName_: string;
  private profileLabel_: string;

  constructor() {
    super();

    this.graduationMojoProvider_ = getGraduationHandlerProvider();
  }

  override connectedCallback(): void {
    super.connectedCallback();

    if (this.isAccountManagerEnabled_) {
      // If we have the Google Account manager, use GAIA name and icon.
      this.addWebUiListener(
          'accounts-changed', this.updateAccounts_.bind(this));
      this.updateAccounts_();
    } else if (this.isFydeProfile_) {
      ProfileInfoBrowserProxyImpl.getInstance().getProfileInfo().then(
          this.handleProfileInfo_.bind(this));
      this.addWebUiListener(
          'profile-info-changed', this.handleProfileInfo_.bind(this));
      SyncBrowserProxyImpl.getInstance().getSyncStatus().then(
          this.handleSyncStatus_.bind(this));
      this.addWebUiListener(
          'sync-status-changed', this.handleSyncStatus_.bind(this));
    }

    this.graduationObserverReceiver_ = new GraduationObserverReceiver(this);
    this.graduationMojoProvider_.addObserver(
        this.graduationObserverReceiver_.$.bindNewPipeAndPassRemote());
  }

  /**
   * Handler for when the account list is updated.
   */
  private async updateAccounts_(): Promise<void> {
    const accounts =
        await AccountManagerBrowserProxyImpl.getInstance().getAccounts();
    this.accounts_ = accounts;

    // The user might not have any GAIA accounts (e.g. guest mode or Active
    // Directory). In these cases the profile row is hidden, so there's nothing
    // to do.
    if (accounts.length === 0) {
      return;
    }

    // Device account is always first per account_manager_ui_handler.cc.
    // TODO(b/325142618) Investigate why `isDeviceAccount` is not always true.
    this.deviceAccount_ = accounts[0];
  }

  onGraduationAppUpdated(isAppEnabled: boolean): void {
    this.showGraduationApp_ =
        loadTimeData.getBoolean('isGraduationFlagEnabled') && isAppEnabled;
  }

  private handleProfileInfo_(info: ProfileInfo) {
    this.profileName_ = info.name;
    if (info.iconUrl.startsWith('data:image/png;base64')) {
      this.profileIconUrl_ = convertImageSequenceToPng([info.iconUrl]);
      return;
    }
    this.profileIconUrl_ = info.iconUrl;
  }

  private handleSyncStatus_(syncStatus: SyncStatus): void {
    if (!this.isAccountManagerEnabled_ && syncStatus &&
        syncStatus.signedInState === SignedInState.SYNCING &&
        syncStatus.signedInUsername) {
      this.profileLabel_ = syncStatus.signedInUsername;
    }
  }

  private getIconImageSet_(iconUrl: string): string {
    return getImage(iconUrl);
  }

  private getProfileName_(): string {
    return this.profileName_;
  }

  private onFydeProfileClick_(): void {
    if (loadTimeData.getBoolean('isFydeProfile')) {
      if (loadTimeData.getBoolean('isFydeLocalAccount')) {
        return;
      }
      const baseUrl = loadTimeData.getString('fydeosAccountBaseUrl');
      const url = `${baseUrl}/personalInfo/`;
      window.open(url);
      return;
    }
  }

  private openLocalAccountChangePasswordSystemSettings_(event: CustomEvent<{event: Event}>): void {
    event.detail.event.preventDefault();
    Router.getInstance().navigateTo(routes.LOCK_SCREEN);
  }
}

declare global {
  interface HTMLElementTagNameMap {
    [OsSettingsPeoplePageElement.is]: OsSettingsPeoplePageElement;
  }
}

customElements.define(
    OsSettingsPeoplePageElement.is, OsSettingsPeoplePageElement);
