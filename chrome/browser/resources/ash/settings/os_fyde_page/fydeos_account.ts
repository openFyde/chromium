// os-settings-fydeos-account

import {PolymerElement} from 'chrome://resources/polymer/v3_0/polymer/polymer_bundled.min.js';
import 'chrome://resources/ash/common/cr_elements/cr_icon_button/cr_icon_button.js';
import 'chrome://resources/ash/common/cr_elements/cr_icons.css.js';
import 'chrome://resources/ash/common/cr_elements/icons.html.js';
import 'chrome://resources/polymer/v3_0/iron-flex-layout/iron-flex-layout-classes.js';
import {WebUiListenerMixin} from 'chrome://resources/ash/common/cr_elements/web_ui_listener_mixin.js';
import {I18nMixin} from 'chrome://resources/ash/common/cr_elements/i18n_mixin.js';
import {convertImageSequenceToPng} from 'chrome://resources/ash/common/cr_picture/png.js';
import {getImage} from 'chrome://resources/js/icon.js';
import {loadTimeData} from 'chrome://resources/js/load_time_data.js';
import {ProfileInfo, ProfileInfoBrowserProxyImpl} from '/shared/settings/people_page/profile_info_browser_proxy.js';
import {SignedInState, SyncBrowserProxy, SyncBrowserProxyImpl, SyncStatus} from '/shared/settings/people_page/sync_browser_proxy.js';
import {AccountManagerBrowserProxyImpl} from '../os_people_page/account_manager_browser_proxy.js';
import '../settings_shared.css.js';
import {getTemplate} from './fydeos_account.html.js';

const FydeSettingsAccountPageElementBase =
    WebUiListenerMixin(I18nMixin(PolymerElement));

/** @polymer */
class FydeSettingsAccountPageElement extends FydeSettingsAccountPageElementBase {
  static get is() {
    return 'os-settings-fydeos-account';
  }

  static get template() {
    return getTemplate();
  }

  static get properties() {
    return {
      isAccountManagerEnabled_: {
        type: Boolean,
        value() {
          return loadTimeData.getBoolean('isAccountManagerEnabled');
        },
        readOnly: true,
      },
      profileIconUrl_: String,
      profileName_: String,
      profileLabel_: String,
    };
  }

  private syncBrowserProxy_: SyncBrowserProxy;
  private isAccountManagerEnabled_: boolean;
  private profileIconUrl_: string;
  private profileName_: string;
  private profileLabel_: string;

  constructor() {
    super();
    this.syncBrowserProxy_ = SyncBrowserProxyImpl.getInstance();
  }

  override connectedCallback() {
    super.connectedCallback();
    if (this.isAccountManagerEnabled_) {
      this.addWebUiListener(
          'accounts-changed', this.updateAccounts_.bind(this));
      this.updateAccounts_();
    } else {
      ProfileInfoBrowserProxyImpl.getInstance().getProfileInfo().then(
          this.handleProfileInfo_.bind(this));
      this.addWebUiListener(
          'profile-info-changed', this.handleProfileInfo_.bind(this));
    }

    this.syncBrowserProxy_.getSyncStatus().then(
        this.handleSyncStatus_.bind(this));
    this.addWebUiListener(
        'sync-status-changed', this.handleSyncStatus_.bind(this));
  }

  getIconImageSet_(iconUrl: string) {
    return getImage(iconUrl);
  }

  onAccountTap_() {
    const baseUrl = loadTimeData.getString('fydeosAccountBaseUrl');
    const url = `${baseUrl}/personalInfo/`;
    window.open(url);
  }

  handleProfileInfo_(info: ProfileInfo) {
    this.profileName_ = info.name;
    if (info.iconUrl.startsWith('data:image/png;base64')) {
      this.profileIconUrl_ = convertImageSequenceToPng([info.iconUrl]);
      return;
    }
    this.profileIconUrl_ = info.iconUrl;
  }

  handleSyncStatus_(syncStatus: SyncStatus) {
    if (syncStatus && syncStatus.signedInState === SignedInState.SYNCING && syncStatus.signedInUsername) {
      this.profileLabel_ = syncStatus.signedInUsername;
    }
  }

  async updateAccounts_() {
    const accounts = await AccountManagerBrowserProxyImpl.getInstance().getAccounts();
    if (accounts.length == 0) {
      return;
    }
    this.profileName_ = accounts[0].fullName;
    this.profileIconUrl_ = accounts[0].pic;

    const profileEmail = accounts[0].email;
    this.profileLabel_ = profileEmail;
  }
}

customElements.define(
    FydeSettingsAccountPageElement.is, FydeSettingsAccountPageElement);
