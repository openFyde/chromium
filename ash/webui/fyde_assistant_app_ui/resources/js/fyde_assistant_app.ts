// Copyright 2023 Fyde Innovations

import '../strings.m.js';
import '../css/fyde_assistant_app_shared.css.js';
import 'chrome://resources/cr_elements/cr_lottie/cr_lottie.js';
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
  static get is(): string {
    return 'fyde-assistant-app';
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
    };
  }

  private webview_: HTMLWebviewElement;

  private loaded_: boolean;
  private showLoading_: boolean;
  private showWebview_: boolean;
  private url_: string;
  private origin_: string;
  private debounceTimeout_: number;
  private isFromLauncher_: boolean;
  private isFromBubble_: boolean;
  private isEventBinded_: boolean;
  private systemColors_: SystemColorInfo;

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
          chrome.send('setAssistantBubbleRect', [x, y, width, height]);
        }
        break;
    }
  }

  bindEvents_(): void {
    if (this.isEventBinded_) return;
    this.isEventBinded_ = true;
    if (this.isFromLauncher_) {
      addWebUIListener('query-from-launcher', this.onQueryFromLauncher_.bind(this));
      addWebUIListener('ui-visibility-changed', this.onUiVisibilityChanged_.bind(this));
    } else if (this.isFromBubble_) {
      addWebUIListener('query-from-bubble', this.onQueryFromBubble_.bind(this));
      addWebUIListener('bubble-visibility-changed', this.onBubbleVisibilityChanged_.bind(this));
    }
    addWebUIListener('system-color-changed', this.onSystemColorChanged_.bind(this));
  }

  onWebviewLoaded_(): void {
    this.loaded_ = true;
    this.showLoading_ = false;
    this.showWebview_ = true;
    chrome.send('onFydeAssistantSwaInit');
    this.bindEvents_();
    this.sendMessage({ method: 'init' });
    this.webview_.focus();
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
      this.webview_.contentWindow.postMessage(this.appendSource(data), this.webview_.src);
    } else {
      console.log('this.webview content window is null');
    }
  }

  onQueryFromLauncher_(message: string): void {
    console.log('query-from-launcher', message);
    if (this.isFromLauncher_) {
      this.sendMessage({ method: 'query', message });
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
  }

  onSystemColorChanged_(colors: SystemColorInfo): void {
    this.systemColors_ = colors;
    console.log('this.systemColors_', this.systemColors_);
    this.sendMessage({ method: 'color-change', data: colors });
  }

  static get template(): HTMLTemplateElement {
    return getTemplate();
  }
}

declare global {
  interface HTMLElementTagNameMap {
    'fyde-assistant-app': FydeAssistantAppElement;
  }
}

customElements.define(
    FydeAssistantAppElement.is, FydeAssistantAppElement);
