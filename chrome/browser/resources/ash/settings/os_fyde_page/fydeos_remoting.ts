// os-settings-fydeos-remoting

import {PolymerElement, mixinBehaviors} from 'chrome://resources/polymer/v3_0/polymer/polymer_bundled.min.js';
import 'chrome://resources/ash/common/cr_elements/cr_toggle/cr_toggle.js';
import 'chrome://resources/ash/common/cr_elements/cr_shared_style.css.js';
import 'chrome://resources/polymer/v3_0/iron-icon/iron-icon.js';
import 'chrome://resources/ash/common/cr_elements/cr_link_row/cr_link_row.js';
import {loadTimeData} from 'chrome://resources/js/load_time_data.js';
import {I18nMixin} from 'chrome://resources/ash/common/cr_elements/i18n_mixin.js';
import {Route, routes} from '../router.js';
import {RouteObserverMixin} from '../common/route_observer_mixin.js'
import {ShellClient} from './shell_client.js';

import {getTemplate} from './fydeos_remoting.html.js';

class RemoteHelperClient {
  private shellClient_: ShellClient;

  constructor(element: HTMLElement) {
    this.shellClient_ = new ShellClient(element);
  }

  static get command() {
    return '/usr/share/remote-help/helper.sh';
  }

  async IsSupported() {
    const exists = this.shellClient_.IsFileExist(RemoteHelperClient.command);
    return exists;
  }

  static get RemoteHelperStates() {
    return {
      ENABLED: 'enabled',
      CLOSED: 'closed',
      REQUIRE_RESTART: 'require_restart',
    }
  }

  static get Events() {
    return {
      REMOTE_HELPER_ENABLED: 'remote_helper_enabled',
      REMOTE_HELPER_CLOSED: 'remote_helper_closed',
      REMOTE_HELPER_REQUIRE_RESTART: 'remote_helper_require_restart',
    };
  }

  async ToggleRemoteHelper(enable: boolean) {
    let result;
    if (enable) {
      result = await this.enableRemoteHelper();
    } else {
      result = await this.disableRemoteHelper();
    }
    this.UpdateRemoteHelperStatus();
    return result;
  }

  async enableRemoteHelper() {
    const port = await this.shellClient_.SafeExecForResult(`${RemoteHelperClient.command} start_helper`);
    return port;
  }

  async disableRemoteHelper() {
    await this.shellClient_.SafeExecForResult(`${RemoteHelperClient.command} stop_helper`);
    return '';
  }

  async getRemoteHelperPort() {
    const port = await this.shellClient_.SafeExecForResult('cat /run/fydeos_remote_helper/port');
    return port;
  }

  async UpdateRemoteHelperStatus() {
    const state = await this.shellClient_.SafeExecForResult(`${RemoteHelperClient.command} running_state`);
    if (!state) {
      this.shellClient_.dispatchEvent(RemoteHelperClient.Events.REMOTE_HELPER_CLOSED, undefined);
      return;
    }
    const port = await this.getRemoteHelperPort();
    if (!port) {
      this.shellClient_.dispatchEvent(RemoteHelperClient.Events.REMOTE_HELPER_REQUIRE_RESTART, undefined);
    } else {
      this.shellClient_.dispatchEvent(RemoteHelperClient.Events.REMOTE_HELPER_ENABLED, { detail: { port } });
    }
  }
}

const FydeSettingsRemotingPageElementBase =
  RouteObserverMixin(I18nMixin(PolymerElement));

class FydeSettingsRemotingPageElement extends FydeSettingsRemotingPageElementBase {
  static get is() {
    return 'os-settings-fydeos-remoting' as const;
  }

  static get template() {
    return getTemplate();
  }

  static get properties() {
    return {
      remoteHelperSupported_: {
        type: Boolean,
        value: false,
      },
      remoteHelperState_: {
        type: String,
        value: RemoteHelperClient.RemoteHelperStates.CLOSED,
      },
      remoteHelperPort_: {
        type: String,
        value: '',
      },
      isStarting_: {
        type: Boolean,
        value: false,
      },
    };
  }

  private remoteHelperClient_: RemoteHelperClient;
  private remoteHelperSupported_: boolean;
  private remoteHelperState_: string;
  private remoteHelperPort_: string;
  private isStarting_: boolean;

  constructor() {
    super();
    console.log('os-settings-fydeos-remoting ready');
    this.remoteHelperClient_ = new RemoteHelperClient(this);
    this.checkIsSupported_();
    this.addListeners();
  }

  override connectedCallback() {
    super.connectedCallback();
    console.log('os-settings-fydeos-remoting attached');
  }

  onRemoteDesktopClicked_() {
// <if expr="use_fydeos_com">
    const appId = 'ihdjkfkbpkgagdkpjpifcdeeadaippme';
// </if>
// <if expr="not use_fydeos_com">
    const appId = 'ddagandcjljahndgakdbdahphkljdcdg';
// </if>
    chrome.nativeWindows.create(appId, () => {
      chrome.nativeWindows.getAll(list => {
        const created = list.find(p => p.appId === appId);
        if (!created) {
          const url = loadTimeData.getString('fydeOSRdpUrl');
          // if failed to create app window, fallback to window.open the url as a normal page
          window.open(url, '_blank');
        }
      });
    });
  }

  override currentRouteChanged(currentRoute: Route) {
    if (currentRoute !== routes.FYDEOS) {
      return;
    }
    this.updateRemoteHelperStatus_();
  }

  checkIsSupported_() {
    // this.remoteHelperSupported_ = await this.remoteHelperClient_.IsSupported();
    this.remoteHelperSupported_ = false;
  }

  addListeners() {
    this.addEventListener(RemoteHelperClient.Events.REMOTE_HELPER_CLOSED, this.onRemoteHelperDisabled_.bind(this));
    this.addEventListener(RemoteHelperClient.Events.REMOTE_HELPER_ENABLED, this.onRemoteHelperEnabled_.bind(this));
    this.addEventListener(RemoteHelperClient.Events.REMOTE_HELPER_REQUIRE_RESTART, this.onRemoteHelperRequireRestart_.bind(this));
  }

  remoteHelperChecked_() {
    return this.remoteHelperState_ === RemoteHelperClient.RemoteHelperStates.ENABLED || this.remoteHelperState_ === RemoteHelperClient.RemoteHelperStates.REQUIRE_RESTART;
  }

  async onToggleRemoteHelper_() {
    if (this.remoteHelperChecked_()) {
      await this.disableRemoteHelper_();
    } else if (this.remoteHelperState_ === RemoteHelperClient.RemoteHelperStates.CLOSED) {
      await this.enableRemoteHelper_();
    }
  }

  async enableRemoteHelper_() {
    this.isStarting_ = true;
    this.remoteHelperState_ = RemoteHelperClient.RemoteHelperStates.ENABLED;
    await this.remoteHelperClient_.ToggleRemoteHelper(true);
    this.isStarting_ = false;
  }

  async disableRemoteHelper_() {
    this.remoteHelperPort_ = '';
    this.remoteHelperState_ = RemoteHelperClient.RemoteHelperStates.CLOSED;
    await this.remoteHelperClient_.ToggleRemoteHelper(false);
  }

  shouldShowRemoteHelperPort_() {
    return this.remoteHelperState_ === RemoteHelperClient.RemoteHelperStates.ENABLED && this.remoteHelperPort_;
  }

  getRemoteHelperMessage_() {
    if (this.remoteHelperState_ === RemoteHelperClient.RemoteHelperStates.REQUIRE_RESTART) {
      return this.i18n('fydeosSettingsRemoteHelperRequireRestartMessage');
    }
    return '';
  }

  getPaperTooltipMessage_() {
    if (this.remoteHelperState_ === RemoteHelperClient.RemoteHelperStates.ENABLED) {
      return this.i18n('fydeosSettingsRemoteHelperEnabledMessage');
    }
    return this.i18n('fydeosSettingsRemoteHelperDesc');
  }

  updateRemoteHelperStatus_() {
    if (this.remoteHelperSupported_ && this.remoteHelperClient_) {
      this.remoteHelperClient_.UpdateRemoteHelperStatus();
    }
  }

  onRemoteHelperEnabled_(e: Event) {
    const { detail: { port } } = e as CustomEvent;
    this.remoteHelperPort_ = port;
    this.remoteHelperState_ = RemoteHelperClient.RemoteHelperStates.ENABLED;
  }

  onRemoteHelperDisabled_() {
    this.remoteHelperPort_ = '';
    this.remoteHelperState_ = RemoteHelperClient.RemoteHelperStates.CLOSED;
  }

  onRemoteHelperRequireRestart_() {
    this.remoteHelperPort_ = '';
    this.remoteHelperState_ = RemoteHelperClient.RemoteHelperStates.REQUIRE_RESTART;
  }

}

customElements.define(
    FydeSettingsRemotingPageElement.is, FydeSettingsRemotingPageElement);
