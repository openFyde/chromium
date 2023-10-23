// Copyright 2023 Fyde Innovations. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview Polymer element for data restore screen.
 */

import '//resources/cr_elements/cr_shared_vars.css.js';
import '//resources/cr_elements/cr_input/cr_input.js';
import '//resources/cr_elements/cr_input/cr_input_style.css.js';
import '../../components/common_styles/oobe_common_styles.css.js';
import '../../components/common_styles/oobe_dialog_host_styles.css.js';
import '../../components/buttons/oobe_back_button.js';
import '../../components/buttons/oobe_text_button.js';
import '../../components/dialogs/oobe_adaptive_dialog.js';
import '../../components/dialogs/oobe_loading_dialog.js';
import '../../components/dialogs/oobe_modal_dialog.js';

import {afterNextRender, html, mixinBehaviors, Polymer, PolymerElement} from '//resources/polymer/v3_0/polymer/polymer_bundled.min.js';
import {LoginScreenBehavior, LoginScreenBehaviorInterface} from '../../components/behaviors/login_screen_behavior.js';
import {MultiStepBehavior, MultiStepBehaviorInterface} from '../../components/behaviors/multi_step_behavior.js';
import {OobeDialogHostBehavior} from '../../components/behaviors/oobe_dialog_host_behavior.js';
import {OobeI18nBehavior, OobeI18nBehaviorInterface} from '../../components/behaviors/oobe_i18n_behavior.js';
import {getSelectedTitle, getSelectedValue, SelectListType, setupSelect} from '../../components/oobe_select.js';
import {FydeOSShellClient} from '../../fydeos_shell_client.js';


const DataRestoreScreenState = {
  SELECT_FILE: 'select-file',
  SETUP: 'setup',
  IN_PROGRESS: 'in-progress',
  FAILED: 'failed',
  SUCCESS: 'success',
};

class DataRestoreHelper {
  static Event = {
    RESTORE_SUCCESS: 'restore-success',
    RESTORE_FAILED: 'restore-failed',
    RESTORE_PROGRESS: 'restore-progress',
  };
  static COMMAND  = '/usr/bin/fydeos-backup';
  static generate_key(salt, password) {
    const str = `${salt}${password}`;
    const hash = btoa(str);
    return `B:${hash}`;
  }
  static restore_command(email, salt, password, backupFile) {
    const encodedFilePath = btoa(backupFile);
    if (salt) {
      const key = this.generate_key(salt, password);
      return `${this.COMMAND} restore --new --email ${email} --special-key ${key} --file ${encodedFilePath}`;
    } else {
      return `${this.COMMAND} restore --new --email ${email} --password ${password} --file ${encodedFilePath}`;
    }
  }
  static list_files_command() {
    return `${this.COMMAND} list`;
  }
  static auto_mount_command() {
    return `${this.COMMAND} auto-mount`;
  }
  static unmount_command() {
    return `${this.COMMAND} unmount`;
  }
  static peek_command(file) {
    const encodedFilePath = btoa(file);
    return `${this.COMMAND} peek --file ${encodedFilePath}`;
  }

  constructor(element) {
    this.client_ = new FydeOSShellClient();
    this.element_ = element;
    this.tmpLogFile_ = '';
  }

  auto_mount() {
    try {
      return this.client_.runCommand(DataRestoreHelper.auto_mount_command());
    } catch (e) {
      console.log(e);
      return '';
    }
  }

  unmount() {
    try {
      return this.client_.runCommand(DataRestoreHelper.unmount_command());
    } catch (e) {
      console.log(e);
      return '';
    }
  }

  async list_files() {
    let raw = ''
    try {
      raw = await this.client_.runCommand(DataRestoreHelper.list_files_command());
    } catch (e) {
      console.log(e);
      return [];
    }
    try {
      return JSON.parse(raw);
    } catch (e) {
      console.log(e);
      return [];
    }
  }

  fire_success_(message) {
    this.element_.dispatchEvent(new CustomEvent(
      DataRestoreHelper.Event.RESTORE_SUCCESS,
      { detail: { message } }
    ));
  }

  fire_error_(error) {
    console.log('error', error);
    this.element_.dispatchEvent(new CustomEvent(
      DataRestoreHelper.Event.RESTORE_FAILED,
      { detail: { error } }
    ));
  }

  fire_progress_(message) {
    this.element_.dispatchEvent(new CustomEvent(
      DataRestoreHelper.Event.RESTORE_PROGRESS,
      { detail: { message } }
    ));
  }

  async getTaskTmpLogFile_(key) {
    try {
      const res = await this.client_.getTaskState(key);
      const { tmpFile } = res;
      this.tmpLogFile_ = tmpFile;
    } catch (e) {
      console.log(e);
    }
  }

  async getLog() {
    if (!this.tmpLogFile_) {
      return '';
    }
    try {
      return await this.client_.runCommand(`cat ${this.tmpLogFile_}`);
    } catch (e) {
      console.log(e);
      return '';
    }
  }

  async peek(file) {
    try {
      return await this.client_.runCommand(DataRestoreHelper.peek_command(file));
    } catch (e) {
      console.log(e);
      return '';
    }
  }

  async restore(email, salt, password, backupFile) {
    let key = '';
    try {
      key = await this.client_.runCommandAsync(DataRestoreHelper.restore_command(email, salt, password, backupFile));
    } catch (e) {
      this.fire_error_(e);
      return;
    }
    await this.getTaskTmpLogFile_(key);
    let ticks = 0;
    const getProgress = async () => {
      ticks++;
      if (ticks > 60 * 20) {
        this.client_.forceCloseTask(key);
        this.fire_error_(new Error('Timeout'));
        return;
      }
      let response = null;
      try {
        response = await this.client_.getOutputOrResult(key);
      } catch (e) {
        this.fire_error_(e);
        return;
      }
      const { closed, result } = response;
      if (closed) {
        this.fire_success_(result);
        return;
      } else {
        this.fire_progress_(result);
        setTimeout(getProgress, 1000);
      }
    };
    getProgress();
  }
}

const DataRestoreScreenElementBase = mixinBehaviors(
  [
    LoginScreenBehavior,
    OobeI18nBehavior,
    OobeDialogHostBehavior,
    MultiStepBehavior,
  ],
  PolymerElement);

class DataRestore extends DataRestoreScreenElementBase {
  static get is() {
    return 'data-restore-element';
  }

  static get template() {
    return html`{__html_template__}`;
  }

  static get properties() {
    return {
      email: {
        type: String,
        value: '',
      },
      emailInvalid: {
        type: Boolean,
        value: false,
      },
      passwordInvalid: {
        type: Boolean,
        value: false,
      },
      selectedBackupFile: {
        type: String,
        value: '',
        observer: 'onSelectedBackupFileChanged_',
      },
      backupFileInvalid: {
        type: Boolean,
        value: false,
      },
    };
  }


  constructor() {
    super();
    const email_local_part_pattern = '[+a-zA-Z0-9_.!#$%&\'*\\/=?^`{|}~-]+';
    const email_domain_part_pattern = '([a-zA-Z0-9-]+\\.)+[a-zA-Z0-9]{2,63}';
    this.validEmailPattern_ = new RegExp(
      `^${email_local_part_pattern}@${email_domain_part_pattern}$`);
    this.helper_ = new DataRestoreHelper(this);
  }

  get EXTERNAL_API() {
    return [
      'setSystemSalt',
      'onUsbDevicesChanged',
    ];
  }

  defaultUIStep() {
    return DataRestoreScreenState.SELECT_FILE;
  }

  get UI_STEPS() {
    return DataRestoreScreenState;
  }

  ready() {
    super.ready();
    console.log('DataRestore ready');
    this.initializeLoginScreen('DataRestoreScreen');
    this.addEventListener(DataRestoreHelper.Event.RESTORE_SUCCESS, this.onRestoreSuccess_.bind(this));
    this.addEventListener(DataRestoreHelper.Event.RESTORE_PROGRESS, this.onRestoreProgress_.bind(this));
    this.addEventListener(DataRestoreHelper.Event.RESTORE_FAILED, this.onRestoreFailed_.bind(this));
    this.salt_ = '';
  }

  setSystemSalt(salt) {
    this.salt_ = salt;
  }

  onUsbDevicesChanged() {
    if (this.uiStep === DataRestoreScreenState.SELECT_FILE) {
      this.prepareBackupFileList_();
    }
  }

  connectedCallback() {
    super.connectedCallback();
    console.log('DataRestore connectedCallback');
  }

  onBeforeShow() {
    this.cleanup_();
    this.prepareBackupFileList_();
  }

  async prepareBackupFileList_() {
    if (this.preparingBackupFileList_) {
      return;
    }
    this.preparingBackupFileList_ = true;
    await this.helper_.auto_mount();
    const files = await this.helper_.list_files();
    this.setupBackupFileList_(files);
    this.preparingBackupFileList_ = false;
  }

  showStep(step) {
    this.setUIStep(step);
  }

  cleanup_() {
    this.email = '';
    this.emailInvalid = false;
    this.password = '';
    this.passwordInvalid = false;
    this.$.dataRestoreEmailInput.value = '';
    this.$.dataRestorePasswordInput.value = '';
    this.selectedBackupFile = '';
    this.backupFileInvalid = false;
    this.serviceLogs = '';
  }

  onBackToSelectFile_() {
    this.setUIStep(DataRestoreScreenState.SELECT_FILE);
  }

  onBack_() {
    this.cleanup_();
    this.helper_.unmount();
    this.userActed('data-restore-exit');
  }

  setupBackupFileList_(files) {
    const fileList = [];
    let title = '';
    let value = '';
    for (const item of files) {
      const { dir, list } = item;
      fileList.push({ optionGroupName: dir });
      const sorted = list.sort((a, b) => {
        const x = parseInt(a.timestamp, 10) || 0;
        const y = parseInt(b.timestamp, 10) || 0;
        return y - x;
      });
      for (const file of sorted) {
        const { name, path, size } = file;
        title = size ? `${name} - ${size}` : name;
        value = path;
        fileList.push({ title, value });
      }
    }
    setupSelect(
        this.$.backupFileSelect, fileList, this.onBackupFileSelect_.bind(this));
    const defaultSelectedBackupFile = getSelectedValue(fileList);
    if (!defaultSelectedBackupFile) {
      // this is a trick to avoid oobe_select set a value after refreshing
      // file list wihout setting this.selectedBackupFile
      this.selectedBackupFile = this.selectedBackupFile === null ? '' : null;
    }
  }

  onBackupFileSelect_(value) {
    this.selectedBackupFile = value;
  }

  restoring() {
    this.setUIStep(DataRestoreScreenState.IN_PROGRESS);
    const email = this.$.dataRestoreEmailInput.value;
    const password = this.$.dataRestorePasswordInput.value;
    const backupFile = this.selectedBackupFile;
    const salt = this.salt_;
    this.helper_.restore(email, salt, password, backupFile);
  }

  onRestoreSuccess_(e) {
    console.log('onRestoreSuccess_', e);
    // will restart automatically
  }

  onRestoreProgress_(e) {
    // console.log('onRestoreProgress_', e);
    const { detail: { message } } = e;
    this.serviceLogs += message;
    this.setServiceLogs(this.serviceLogs);
  }

  async onRestoreFailed_(e) {
    console.log('onRestoreFailed_', e);
    this.setUIStep(DataRestoreScreenState.FAILED);
    const { detail: { error } } = e;
    const log = await this.helper_.getLog();
    this.setServiceLogs(log || error.message);
  }

  onKeyDownEmailInput_(e) {
    this.emailInvalid = false;
    if (e.key == 'Enter') {
      this.switchTo_('dataRestorePasswordInput') || this.switchTo_('backupFileSelect') || this.onSubmit_();
    }
  }

  onKeyDownPasswordInput_(e) {
    this.passwordInvalid = false;
    if (e.key == 'Enter') {
      this.switchTo_('backupFileSelect') || this.onSubmit_();
    }
  }

  onSelectedBackupFileChanged_(newValue, oldValue) {
    if (newValue) {
      this.backupFileInvalid = false;
      if (newValue !== oldValue && !this.email) {
        this.peekEmailFromBackupFile_(newValue);
      }
    }
  }

  async peekEmailFromBackupFile_(backupFile) {
    if (this.peeking_) {
      return;
    }
    this.peeking_ = true;
    const ret = await this.helper_.peek(backupFile);
    if (!ret) {
      this.peeking_ = false;
      return;
    }
    let content = '';
    try {
      content = atob(ret);
    } catch (e) {
      console.log('peekEmailFromBackupFile_ failed', e);
      this.peeking_ = false;
      return;
    }

    if (!content) {
      this.peeking_ = false;
      return;
    }

    if (content[0] === '{') {
      try {
        const obj = JSON.parse(content);
        if (obj.email) {
          this.email = obj.email;
        }
      } catch (e) {
        console.log('peekEmailFromBackupFile_ failed', e);
        this.peeking_ = false;
        return;
      }
    } else if (content.length < 256 && content.indexOf('@') !== -1) {
      this.email = content;
    }
    this.peeking_ = false;
  }

  switchTo_(inputId) {
    if (!this.$[inputId].disabled && this.$[inputId].value.length == 0) {
      this.$[inputId].focus();
      return true;
    }
    return false;
  }

  onSubmit_() {
    this.emailInvalid =
      this.validEmailPattern_.test(this.$.dataRestoreEmailInput.value) === false;
    if (this.emailInvalid) {
      return;
    }
    this.passwordInvalid = !this.$.dataRestorePasswordInput.validate();
    if (this.passwordInvalid) {
      return;
    }

    this.restoring();
  }

  onSelectFileNextButtonPressed_() {
    this.backupFileInvalid = !this.selectedBackupFile;
    if (this.backupFileInvalid) {
      return;
    }
    this.setUIStep(DataRestoreScreenState.SETUP);
  }

  onSetupNextButtonPressed_() {
    this.onSubmit_();
  }

  onErrorBackToSetupButtonPressed_() {
    this.setUIStep(DataRestoreScreenState.SETUP);
  }

  onServiceLogsLinkClicked_() {
    this.$.serviceLogsDialog.showDialog();
    this.$.closeServiceLogsDialog.focus();
  }

  hideServiceLogsDialog_() {
    this.$.serviceLogsDialog.hideDialog();
    this.focusLogsLink_();
  }

  focusLogsLink_() {
    if (this.uiStep == DataRestoreScreenState.FAILED) {
      afterNextRender(this, () => this.$.serviceLogsLink.focus());
    }
  }

  setServiceLogs(serviceLogs) {
    this.$.serviceLogsFrame.src = 'data:text/html;charset=utf-8,' +
        encodeURIComponent('<style>' +
                           'body {' +
                           '  font-family: Roboto, sans-serif;' +
                           '  color: RGBA(0,0,0,.87);' +
                           '  font-size: 14sp;' +
                           '  margin : 0;' +
                           '  padding : 0;' +
                           '  white-space: pre-wrap;' +
                           '}' +
                           '#logsContainer {' +
                           '  overflow: auto;' +
                           '  height: 99%;' +
                           '  padding-left: 16px;' +
                           '  padding-right: 16px;' +
                           '}' +
                           '#logsContainer::-webkit-scrollbar-thumb {' +
                           '  border-radius: 10px;' +
                           '}' +
                           '</style>' +
                           '<body><div id="logsContainer">' + serviceLogs +
                           '</div>' +
                           '</body>');
  }
}

customElements.define(DataRestore.is, DataRestore);
