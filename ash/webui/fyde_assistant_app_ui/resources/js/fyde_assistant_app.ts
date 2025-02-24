// Copyright 2023 Fyde Innovations

import '/strings.m.js';
import '../css/fyde_assistant_app_shared.css.js';
import 'chrome://resources/ash/common/cr_elements/cr_lottie/cr_lottie.js';
import {addWebUIListener} from 'chrome://resources/ash/common/cr.m.js';
import {loadTimeData} from 'chrome://resources/ash/common/load_time_data.m.js';
import {PolymerElement} from 'chrome://resources/polymer/v3_0/polymer/polymer_bundled.min.js';
import {PolymerElementProperties} from 'chrome://resources/polymer/v3_0/polymer/interfaces.js';
import {getTemplate} from './fyde_assistant_app.html.js';

interface ContextMenus {
  onShow: {
    addListener(callback: (e: Event) => void): void;
  };
}

interface WebviewLoadAbortEvent extends Event {
  url: string;
  isTopLevel: boolean;
}

interface WebviewNewWindowEvent extends Event {
  partition: string;
  targetUrl: string;
}

interface HTMLWebviewElement extends HTMLElement {
  src: string;
  partition: string;
  contentWindow: Window;
  contextMenus: ContextMenus;
}

interface MessageData {
  method: string;
  message?: string;
  data?: any;
  source?: string;
}

interface SystemColorInfo {
  base: string;
  shaded: string;
  header: string;
  primary: string;
}

const FYDE_ASSISTANT_APP_WEBVIEW_PARTITION = 'persist:fydeosAssistant';

const FydeAssistantAppElementBase = PolymerElement;

export class FydeAssistantAppElement extends FydeAssistantAppElementBase {
  static get is() {
    return 'fyde-assistant-app' as const;
  }

  static get properties(): PolymerElementProperties {
    return {
      loaded_: {
        type: Boolean,
        value: false,
      },
      showLoading_: {
        type: Boolean,
        value: false,
      },
      showWebview_: {
        type: Boolean,
        value: false,
      },
      loadAbort_: {
        type: Boolean,
        value: false,
      },
    };
  }

  private webview_: HTMLWebviewElement;

  private loaded_: boolean;
  private loadAbort_: boolean;
  private showLoading_: boolean;
  private showWebview_: boolean;
  private url_: string;
  private origin_: string;
  private debounceTimeout_: number;
  private isFromLauncher_: boolean;
  private isFromBubble_: boolean;
  private isEventBinded_: boolean;
  private systemColors_: SystemColorInfo;
  private files_: File[];
  private latestNetworkType_: number;
  private isBubbleVisible_: boolean;

  constructor() {
    super();
    this.loaded_ = false;
    this.url_ = '';
    this.origin_ = '';
    this.debounceTimeout_ = 200;
    this.isFromLauncher_ = false;
    this.isFromBubble_ = false;
    this.isEventBinded_ = false;
    this.systemColors_ = { base: '', shaded: '', header: '', primary: '' };
    this.files_ = [];
    this.latestNetworkType_ = 0;
    this.isBubbleVisible_ = false;
    if (window.launchQueue) {
      window.launchQueue.setConsumer(this.consumerFiles.bind(this));
    }
    if(!loadTimeData.getBoolean('isFydeOSAssistantEnabled')) {
      window.open('chrome://os-settings/osFydeAssistant');
      window.close();
    }
    if(!loadTimeData.getBoolean('isFydeOSAssistantEnabled')) {
      window.open('chrome://os-settings/osFydeAssistant');
      window.close();
    }
  }

  async consumerFiles(launchParams: LaunchParams) {
    const handles = launchParams.files as FileSystemFileHandle[];
    const promises = handles.map(async (handle) => {
      const file = await handle.getFile();
      return file;
    });
    this.files_ = await Promise.all(promises);
    if (this.loaded_) {
      this.sendMessage({ method: 'files', data: { files: this.files_ } });
    }
  }

  override ready(): void {
    super.ready();
    this.webview_ = this.shadowRoot!.querySelector('#webview') as HTMLWebviewElement;
    const params = new URLSearchParams(document.location.search);
    const source = params.get('source');
    this.isFromLauncher_ = (source === 'launcher');
    this.isFromBubble_ = (source === 'bubble');
    this.tweakContainerStyle_();
  }

  override connectedCallback(): void {
    super.connectedCallback();

    window.addEventListener('message', this.onMessage_.bind(this));

    this.url_ = this.getUrl_();
    this.origin_ = new URL(this.url_).origin;

    this.webview_.partition = FYDE_ASSISTANT_APP_WEBVIEW_PARTITION;
    this.webview_.contextMenus.onShow.addListener((e) => {
      e.preventDefault();
    });
    setTimeout(() => {
      if (this.loaded_) {
        return;
      }
      this.showLoading_ = true;
    }, this.debounceTimeout_);
    this.webview_.src = this.url_;
    this.webview_.addEventListener('contentload', this.onWebviewLoaded_.bind(this));
    this.webview_.addEventListener('loadabort', this.onWebviewAborted_.bind(this));
    this.webview_.addEventListener('newwindow', this.onNewWindow_.bind(this));

    this.bindNetworkEvents_();
  }

  tweakContainerStyle_() {
    if (!this.isFromLauncher_ && !this.isFromBubble_) return;
    let borderRadius = 0;
    if (this.isFromLauncher_) {
      if (!loadTimeData.valueExists('borderRadiusInLauncher')) return;
      borderRadius = loadTimeData.getInteger('borderRadiusInLauncher');
    } else if (this.isFromBubble_) {
      if (!loadTimeData.valueExists('borderRadiusInBubble')) return;
      borderRadius = loadTimeData.getInteger('borderRadiusInBubble');
    }
    const e = this.shadowRoot!.querySelector('#container') as HTMLElement;
    if (e) e.style.borderRadius = `${borderRadius}px`;
  }

  appendValueToUrl_(url: string, key: string) {
    if (loadTimeData.valueExists(key)) {
      return `${url}&${key}=${encodeURIComponent(loadTimeData.getString(key))}`;
    }
    return url;
  }

  appendColorToUrl_(url: string, key: string) {
    if (this.systemColors_[key as keyof SystemColorInfo]) {
      return `${url}&${key}_color=${encodeURIComponent(this.systemColors_[key as keyof SystemColorInfo])}`;
    }
    return this.appendValueToUrl_(url, `${key}_color`);
  }

  getUrl_() {
    let url = loadTimeData.getString('fydeosAssistantUrl');
    url = url.endsWith('/') ? url.slice(0, -1) : url;
    const path = document.location.pathname;
    url = `${url}${path}`;
    url = `${url}${document.location.search || '?'}`;
    url = this.appendValueToUrl_(url, 'user');
    const keys = ['base', 'shaded', 'header', 'primary'];
    for (let i = 0; i < keys.length; i++) {
      url = this.appendColorToUrl_(url, keys[i]);
    }
    return url;
  }

  removeInitQueryParam_(url: string): string {
    try {
      const urlObject = new URL(url);
      const searchParams = new URLSearchParams(urlObject.search);

      if (searchParams.has('initQuery')) {
        searchParams.delete('initQuery');
      }

      const newSearchParams = searchParams.toString();
      const newURL = new URL(`${urlObject.origin}${urlObject.pathname}?${newSearchParams}`);
      return newURL.href;
    } catch (error) {
      return url;
    }
  }

  reloadWebview_(reloadParams: { keepInitQuery: boolean }) {
    this.loadAbort_ = false;
    this.files_ = [];
    this.url_ = this.getUrl_();
    if (reloadParams && reloadParams.keepInitQuery === false) {
      this.url_ = this.removeInitQueryParam_(this.url_);
    }
    this.webview_.src = this.url_;
  }

  onMessage_(e: MessageEvent): void {
    const webviewWindow = this.webview_.contentWindow;
    if (!(!!webviewWindow && webviewWindow === e.source)) {
      console.log('invalid message source', e.source);
      return;
    }
    if (e.origin !== this.origin_) {
      console.log('invalid message origin', e.origin);
      return;
    }

    if (!e.data) {
      console.log('invalid message format', e.data);
      return;
    }

    console.log('received message', e.data);
    const { method, message, data } = e.data;

    switch (method) {
      case 'close':
        if (this.isFromLauncher_) {
          chrome.send('onCloseAssistant', ['launcher']);
        } else if (this.isFromBubble_) {
          chrome.send('onCloseAssistant', ['bubble']);
        }
        break;
      case 'reload':
        this.reloadWebview_(data);
        break;
      case 'openUrl':
        const { url } = message;
        if (url) {
          chrome.send('onOpenAssistantUrl', [url]);
        }
        break;
      case 'setRect':
        if (data) {
          const { x=0, y=0, width=0, height=0 } = data;
          chrome.send('setAssistantBubbleRect', [Math.round(x), Math.round(y), Math.round(width), Math.round(height)]);
        }
        break;
      case 'centerBubble':
        if (data) {
          const { width=0, height=0 } = data;
          chrome.send('centerAssistantBubble', [Math.round(width), Math.round(height)]);
        }
        break;
    }
  }

  bindNetworkEvents_(): void {
    addWebUIListener('network-connection-changed', this.onNetworkConnectionChanged_.bind(this));
  }

  bindEvents_(): void {
    if (this.isEventBinded_) return;
    this.isEventBinded_ = true;
    if (this.isFromBubble_) {
      addWebUIListener('query-from-bubble', this.onQueryFromBubble_.bind(this));
      addWebUIListener('bubble-visibility-changed', this.onBubbleVisibilityChanged_.bind(this));
    } else {
      addWebUIListener('query-from-launcher', this.onQueryFromLauncher_.bind(this));
      // addWebUIListener('ui-visibility-changed', this.onUiVisibilityChanged_.bind(this));
    }
    addWebUIListener('system-color-changed', this.onSystemColorChanged_.bind(this));
  }

  onWebviewLoaded_(): void {
    chrome.send('onFydeAssistantSwaInit');
    if (this.loadAbort_) {
      return;
    }
    this.loaded_ = true;
    this.showLoading_ = false;
    this.showWebview_ = true;
    this.bindEvents_();
    this.sendMessage({ method: 'init', data: { files: this.files_ } });
    this.webview_.focus();
  }

  onWebviewAborted_(e: Event): void {
    if (this.loaded_) {
      return;
    }
    const { isTopLevel, url } = e as WebviewLoadAbortEvent;
    if (isTopLevel && url && url.startsWith(this.origin_)) {
      this.loadAbort_ = true;
    }
  }

  onNewWindow_(e: Event): void {
    const { partition, targetUrl } = e as WebviewNewWindowEvent;
    if (partition !== FYDE_ASSISTANT_APP_WEBVIEW_PARTITION) {
      return;
    }
    e.preventDefault();
    window.open(targetUrl, '_blank');
  }

  appendSource(data: MessageData) {
    let source = 'unknown';
    if (this.isFromLauncher_) {
      source = 'launcher';
    } else if (this.isFromBubble_) {
      source = 'bubble';
    }
    return { ...data, source };
  }

  sendMessage(data: MessageData) {
    if (this.webview_.contentWindow) {
      console.log('sending message', this.appendSource(data));
      this.webview_.contentWindow.postMessage(this.appendSource(data), this.webview_.src);
    } else {
      console.log('this.webview content window is null');
    }
  }

  onQueryFromLauncher_(message: string): void {
    console.log('query-from-launcher', message);
    if (!this.isFromBubble_) {
      // option one:
      // this.sendMessage({ method: 'query', message });

      // option two: reload with new initQuery
      this.url_ = this.removeInitQueryParam_(this.url_);
      this.webview_.src = `${this.url_}&initQuery=${encodeURIComponent(message)}`;
    }
  }

  onUiVisibilityChanged_(visible: boolean): void {
    this.sendMessage({ method: 'visibility-change', data: visible });
  }

  onQueryFromBubble_(data: { query: string, format: number }): void {
    if (this.isFromBubble_) {
      this.sendMessage({ method: 'bubble-query', data });
    }
  }

  onBubbleVisibilityChanged_(visible: boolean): void {
    this.sendMessage({ method: 'bubble-visibility-change', data: visible });
    this.isBubbleVisible_ = visible;
  }

  onSystemColorChanged_(colors: SystemColorInfo): void {
    this.systemColors_ = colors;
    console.log('this.systemColors_', this.systemColors_);
    this.sendMessage({ method: 'color-change', data: colors });
  }

  onNetworkConnectionChanged_(type: number): void {
    if (type === this.latestNetworkType_) {
      return;
    }
    const shouldReload = this.shouldReloadWebviewAfterNetworkConnectionChanged_(type);
    if (shouldReload) {
      console.log('reload webview after network connection changed');
      this.reloadWebview_({ keepInitQuery: false });
    }
  }

  shouldReloadWebviewAfterNetworkConnectionChanged_(connectionType: number): boolean {
    const latestNetworkType = this.latestNetworkType_;
    this.latestNetworkType_ = connectionType;

    // services/network/public/mojom/network_change_manager.mojom
    if (latestNetworkType !== 6) {
      return false;
    }

    // from offline -> online
    return this.loadAbort_ || (this.isFromBubble_ && !this.isBubbleVisible_);
  }

  static get template(): HTMLTemplateElement {
    return getTemplate();
  }
}

declare global {
  interface HTMLElementTagNameMap {
    [FydeAssistantAppElement.is]: FydeAssistantAppElement;
  }
}

customElements.define(
    FydeAssistantAppElement.is, FydeAssistantAppElement);
