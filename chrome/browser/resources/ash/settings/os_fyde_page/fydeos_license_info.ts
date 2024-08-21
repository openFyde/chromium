// os-settings-fydeos-license-info

import {PolymerElement} from 'chrome://resources/polymer/v3_0/polymer/polymer_bundled.min.js';
import 'chrome://resources/ash/common/cr_elements/cr_button/cr_button.js';
import 'chrome://resources/polymer/v3_0/paper-spinner/paper-spinner-lite.js';
import '../settings_shared.css.js';
import {I18nMixin} from 'chrome://resources/ash/common/cr_elements/i18n_mixin.js';
import {loadTimeData} from 'chrome://resources/js/load_time_data.js';
import {ShellClient} from './shell_client.js';

import {getTemplate} from './fydeos_license_info.html.js';


interface WebviewLoadAbortEvent extends Event {
  url: string;
  isTopLevel: boolean;
}

interface ContextMenus {
  onShow: {
    addListener(callback: (e: Event) => void): void;
  };
}

interface HTMLWebViewElement extends HTMLIFrameElement {
  contentWindow: Window;
  contextMenus: ContextMenus;
}

interface DataFromWebview {
  method: string;
  url: string;
}

class LicenseInfoClient {
  private shellClient_: ShellClient;
  private isDebug: boolean;

  constructor(element: HTMLElement) {
    this.isDebug = false;
    this.shellClient_ = new ShellClient(element);
  }

  async GetLicenseID() {
    if (this.isDebug) return 'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa';
    const ret = await this.shellClient_.SafeExecForResult('/usr/share/fydeos_shell/license-utils.sh id');
    return ret;
  }

  async GetSerialNum() {
    if (this.isDebug) return 'ffffffffffff';
    const ret = await this.shellClient_.SafeExecForResult('vpd -g serial_number');
    return ret;
  }

async GetLicenseExpireDate() {
    if (this.isDebug) return '2020-04-29T04:59:16.965Z';
    const text = await this.shellClient_.SafeExecForResult('/usr/share/fydeos_shell/license-utils.sh read');
    if (!text) return '';
    let data;
    try {
      data = JSON.parse(text);
    } catch (err) {
      return '';
    }
    if (data.expire_date) { return data.expire_date; }
    return '';
  }
}

const urlOpenAllowOriginList = [
  'https://cashier.fydeos.io',
  'https://cashier.fydeos.com',
];

const FydeSettingsLicenseInfoPageElementBase = I18nMixin(PolymerElement);

/** @polymer */
class FydeSettingsLicenseInfoPageElement extends FydeSettingsLicenseInfoPageElementBase {
  static get is() {
    return 'os-settings-fydeos-license-info';
  }
  static get template() {
    return getTemplate();
  }

  static get properties() {
    return {
      isLoading: {
        type: Boolean,
        value: false,
      },
      getIdError: {
        type: Boolean,
        value: false,
      },
      isOffline: {
        type: Boolean,
        value: false,
      },
      errorMessage: {
        type: String,
        value: '',
      },
      loadingText: {
        type: String,
        value: '',
      },
      licenseID: {
        type: String,
        value: '',
      },
      expireDate: {
        type: String,
        value: '',
      },
    };
  }

  private isLoading: boolean;
  private getIdError: boolean;
  private isOffline: boolean;
  private errorMessage: string;
  private loadingText: string;
  private licenseID: string;
  private expireDate: string;

  private FYDEOS_LICENSE_INFO_URL: string;
  private isWebviewUrlSet: boolean;
  private shellClient: typeof chrome.shellClient;
  private serialNumber: string;
  private licenseInfoClient_: LicenseInfoClient;
  private channelId: number;

  private webview_: HTMLWebViewElement;

  private fydeosSettingsLicenseUrl: string;
  private licenseOrigin: string;
  private licenseAccountQueryString: string;


  constructor() {
    super();
    this.fydeosSettingsLicenseUrl = loadTimeData.getString('fydeosSettingsLicenseUrl');
    const licenseUrl = new URL(this.fydeosSettingsLicenseUrl);
    this.licenseOrigin = licenseUrl.origin;
    this.licenseAccountQueryString = licenseUrl.search.substr(1);

    this.isWebviewUrlSet = false;
    this.shellClient = chrome.shellClient;
    this.serialNumber = '';
  }

  override ready() {
    super.ready();
    this.licenseInfoClient_ = new LicenseInfoClient(this);

    this.webview_ = this.shadowRoot!.getElementById('license-info')! as HTMLWebViewElement;
    console.log('os-settings-license-info ready, webview_: ', this.webview_);

    this.bindWebViewEvents_();
    this.channelId = (new Date()).getTime();
  }

  override connectedCallback() {
    super.connectedCallback();
    console.log('os-settings-license-info attached');
    if (!this.webview_) return;
    this.webview_.contextMenus.onShow.addListener((e: Event) => {
      e.preventDefault();
    });
    this.load_();
  }

  bindWebViewEvents_() {
    if (!this.webview_) return;

    this.webview_.addEventListener('contentload', this.onContentLoad_.bind(this));
    this.webview_.addEventListener('loadabort', this.onLoadAbort_.bind(this));
    // this.webview_.addEventListener('loadcommit', this.onLoadCommit_.bind(this));
    window.addEventListener('message', this.onMessage_.bind(this));

    // this.webview_.request.onCompleted.addListener(
    //   this.onRequestCompleted_.bind(this),
    //   { urls: ['<all_urls>'], types: ['main_frame']},
    //   ['responseHeaders'],
    // );

    // this.webview_.request.onHeadersReceived.addListener(
    //   this.onHeadersReceived_.bind(this),
    //   {urls: ['<all_urls>'], types: ['main_frame', 'xmlhttprequest']},
    //   ['responseHeaders'],
    // );
  }

  async load_() {
    this.setLoadingState_('');
    this.licenseID = await this.licenseInfoClient_.GetLicenseID();
    this.serialNumber = await this.licenseInfoClient_.GetSerialNum();
    const expireDate = await this.licenseInfoClient_.GetLicenseExpireDate();
    this.expireDate = this.formatDate(expireDate);
    if (this.licenseID) {
      this.webview_.src = this.appendExtraParamsForUrl_(`${this.fydeosSettingsLicenseUrl}&licenseId=${this.licenseID}&serialNumber=${this.serialNumber}`);
      this.isWebviewUrlSet = true;
    } else {
      this.isLoading = false;
      this.getIdError = true;
      this.errorMessage = this.i18n('fydeosSettingsLicenseErrorReadMachineId');
    }
  }

  formatDate(dateString: string) {
    if (!dateString) return '';
    let date;
    try {
      date = new Date(dateString);
    } catch (err) {
      return '';
    }
    if (Number.isNaN(date.getTime())) {
      return '';
    }
    return date.toLocaleString();
  }

  onContentLoad_() {
    this.isLoading = false;
    this.setupTunnel();
  }

  onLoadAbort_(e: Event) {
    const { isTopLevel, url } = e as WebviewLoadAbortEvent;
    if (!isTopLevel || !url.startsWith(this.licenseOrigin)) {
      return;
    }
    if (this.isWebviewUrlSet) {
      this.isLoading = false;
      this.isOffline = true;
    }
  }

  // onLoadCommit_: function(e) {
    // console.log('onLoadCommit_', e);
  // },

  // onRequestCompleted_: function(e) {
    // console.log('onRequestCompleted_', e);
  // },

  // onHeadersReceived_: function(e) {
    // console.log('onHeadersReceived_', e);
  // },

  getFrameContainerClass_(isLoading: boolean, isOffline: boolean) {
    if (isOffline) return 'hidden';
    return isLoading ? 'transparent' : 'non-transparent';
  }

  getLoadingContainerClass_(isLoading: boolean, isOffline: boolean) {
    return (isLoading && !isOffline) ? 'step-loading' : 'hidden';
  }

  setupTunnel() {
    const email = loadTimeData.valueExists('primaryUserEmail') ? loadTimeData.getString('primaryUserEmail') : '';
    const ownerEmail = loadTimeData.valueExists('ownerEmail') ? loadTimeData.getString('ownerEmail') : '';
    if (this.webview_.contentWindow) {
      this.webview_.contentWindow.postMessage({
        type: 'fydeos-license-info-channel-message',
        channelId: this.channelId,
        payload: {
          name: 'fydeos-license-info-internal-reply-message',
          extra: {
            email,
            // expireDate: this.expireDate,
          },
        },
      }, this.licenseOrigin);
    }
  }

  onMessage_(message: MessageEvent) {
    const { data, origin } = message;
    if (origin !== this.licenseOrigin) return;
    const { channelId, type, payload } = data;
    if (type === 'fydeos-license-info-channel-message' && channelId === this.channelId) {
      this.onChannelMessage_(payload);
    } else {
      this.onNormalMessage_(data as DataFromWebview);
    }
  }

  onChannelMessage_(payload: object) {
    // NOTE most work would be here, NOT ANY MORE
    // since settings do not save license
    // data might be { name: 'setLicense', data: { license: 'xxxxx', expire: '2020-10-01' } }
    console.log('channel message', payload);
    return;
  }

  // validateLicense_(payload) {
  //   const { license, expire } = payload;
  //   return !!license && !!expire;
  // },

  onNormalMessage_(data: DataFromWebview) {
    // NOTE might be the message told this element to show loading ui
    // console.log('normal message', data);
    const { method, url } = data;
    if (method === 'openWindow') {
      if (!url || !this.canOpenWindow_(url)) {
        window.open(this.appendExtraParamsForUrl_(`${this.licenseOrigin}/web/checkoutCounter.html?licenseId=${this.licenseID}&${this.licenseAccountQueryString}`));
      } else {
        window.open(url);
      }
    }
  }

  canOpenWindow_(url: string) {
    return url && (url.startsWith(this.licenseOrigin)
                   || urlOpenAllowOriginList.find(item => url.startsWith(item)));
  }

  appendExtraParamsForUrl_(url: string) {
    const lang = loadTimeData.getString('language');
    const board = loadTimeData.getString('fydeosBoardName');
    return `${url}&board=${board}&hl=${lang}`;
  }

  setLoadingState_(loadingText: string) {
    this.isLoading = true;
    this.loadingText = loadingText || this.i18n('fydeosSettingsLicenseStateLoading');
  }

  resetState_() {
    this.isLoading = false;
    this.getIdError = false;
    this.isOffline = false;
    this.isWebviewUrlSet = false;
    this.errorMessage = '';
    this.loadingText = '';
  }

  onRetryClick_() {
    this.resetState_();
    this.load_();
  }

  shouldShowOffline_(isOffline: boolean, licenseID: string) {
    return isOffline && licenseID;
  }
}

customElements.define(
    FydeSettingsLicenseInfoPageElement.is, FydeSettingsLicenseInfoPageElement);
