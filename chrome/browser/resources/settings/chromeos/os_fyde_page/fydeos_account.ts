// os-settings-fydeos-account

import {PolymerElement} from 'chrome://resources/polymer/v3_0/polymer/polymer_bundled.min.js';
import 'chrome://resources/cr_elements/cr_icon_button/cr_icon_button.js';
import 'chrome://resources/cr_elements/cr_icons.css.js';
import 'chrome://resources/cr_elements/icons.html.js';
import 'chrome://resources/polymer/v3_0/iron-flex-layout/iron-flex-layout-classes.js';
import {CrToggleElement} from 'chrome://resources/cr_elements/cr_toggle/cr_toggle.js';
import {WebUiListenerMixin} from 'chrome://resources/cr_elements/web_ui_listener_mixin.js';
import {I18nMixin} from 'chrome://resources/cr_elements/i18n_mixin.js';
import {sendWithPromise} from 'chrome://resources/js/cr.js';
import {convertImageSequenceToPng} from 'chrome://resources/ash/common/cr_picture/png.js';
import {getImage} from 'chrome://resources/js/icon.js';
import {loadTimeData} from 'chrome://resources/js/load_time_data.js';
import {ProfileInfo, ProfileInfoBrowserProxyImpl} from '/shared/settings/people_page/profile_info_browser_proxy.js';
import {SyncBrowserProxy, SyncBrowserProxyImpl, SyncStatus} from '/shared/settings/people_page/sync_browser_proxy.js';
import {AccountManagerBrowserProxyImpl} from '../os_people_page/account_manager_browser_proxy.js';
import '../../settings_shared.css.js';
import '../../controls/password_prompt_dialog.js';
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
      isProfileActionable_: {
        type: Boolean,
        value() {
          return loadTimeData.getBoolean('isFydeProfile') && !loadTimeData.getBoolean('isFydeLocalAccount');
        }
      },

      isFydeLocalAccount_: {
        type: Boolean,
        value() {
          return loadTimeData.getBoolean('isFydeLocalAccount');
        },
      },

      systemSaltObtained_: {
        type: Boolean,
        value: false,
      },

      isOfflineAutoSigninEnabled_: {
        type: Boolean,
        value: false,
      },

      isOfflineAutoSigninEnabledForCurrentUser_: {
        type: Boolean,
        value: false,
      },

      showPasswordPromptDialog_: {
        type: Boolean,
        value: false,
      },

      profileIconUrl_: String,
      profileName_: String,
      profileLabel_: String,
    };
  }

  private syncBrowserProxy_: SyncBrowserProxy;
  private isAccountManagerEnabled_: boolean;
  private isProfileActionable_: boolean;
  private isFydeLocalAccount_: boolean;
  private systemSaltObtained_: boolean;
  private isOfflineAutoSigninEnabled_: boolean;
  private isOfflineAutoSigninEnabledForCurrentUser_: boolean;
  private showPasswordPromptDialog_: boolean;
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
    this.addWebUiListener('offline-auto-signin-system-salt-obtained',
        this.onOfflineAutoSigninSystemSaltObtained_.bind(this));
    this.getIsOfflineAutoSigninEnabled_();
  }

  getIsOfflineAutoSigninEnabled_() {
    sendWithPromise('getIsOfflineAutoSigninEnabled').then((result) => {
      console.log('getIsOfflineAutoSigninEnabled_', result);
      const { is_current_user, enabled, system_salt_obtained } = result;
      this.isOfflineAutoSigninEnabled_ = enabled;
      this.isOfflineAutoSigninEnabledForCurrentUser_ = is_current_user;
      this.systemSaltObtained_ = system_salt_obtained;
    }).finally(() => {
      const ele = this.shadowRoot!.querySelector('#toggleOfflineAutoSignin') as CrToggleElement;
      ele.checked = this.getOfflineAutoSigninCheckedState_();
    });
  }

  getIconImageSet_(iconUrl: string) {
    return getImage(iconUrl);
  }

  onAccountTap_() {
    if (!this.isProfileActionable_) return;
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
    if (syncStatus && syncStatus.signedIn && syncStatus.signedInUsername) {
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

  onToggleOfflineAutoSignin_() {
    if (this.getOfflineAutoSigninCheckedState_()) {
      sendWithPromise('cleanOfflineLoginPassword').then((result) => {
        console.log('cleanOfflineLoginPassword result', result);
        this.getIsOfflineAutoSigninEnabled_();
      });
    } else {
      this.showPasswordPromptDialog_ = true;
    }
  }

  onPasswordPromptClosed_() {
    this.showPasswordPromptDialog_ = false;
    this.getIsOfflineAutoSigninEnabled_();
  }

  onPasswordExposed_(e: Event) {
    const { detail } = e as CustomEvent;
    const password = detail;
    sendWithPromise('saveOfflineLoginPassword', password).then((result) => {
      console.log('saveOfflineLoginPassword result', result);
    });
  }

  onOfflineAutoSigninSystemSaltObtained_() {
    this.getIsOfflineAutoSigninEnabled_();
  }

  getOfflineAutoSigninCheckedState_() {
    return this.isOfflineAutoSigninEnabled_ && this.isOfflineAutoSigninEnabledForCurrentUser_;
  }

  getOfflineAutoSigninMessage_() {
    const defaultMessage = this.i18n('enableAutoSigninForFydeLocalAccountHelpMessage');
    if (!this.isFydeLocalAccount_) {
      return this.i18n('unableToSetAutoSigninForFydeNonLocalAccount');
    }
    if (this.isOfflineAutoSigninEnabled_) {
      if (this.isOfflineAutoSigninEnabledForCurrentUser_) {
        return defaultMessage;
      }
      else {
        return this.i18n('autoSigninForFydeLocalAccountOtherUserAlreadyEnabled');
      }
    } else {
      if (this.systemSaltObtained_) {
        return defaultMessage;
      } else {
        return this.i18n('unableToSetAutoSigninForFydeLocalAccount');
      }
    }
  }

  getCanToggleAutoSignin_() {
    if (!this.isFydeLocalAccount_) {
      return false;
    }
    if (this.isOfflineAutoSigninEnabled_) {
      return this.isOfflineAutoSigninEnabledForCurrentUser_;
    }
    return this.systemSaltObtained_;
  }

}

customElements.define(
    FydeSettingsAccountPageElement.is, FydeSettingsAccountPageElement);
