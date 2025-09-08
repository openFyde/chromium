// @ts-nocheck

/**
 * @fileoverview Polymer element for OS install screen.
 */


import {html, mixinBehaviors, PolymerElement} from '//resources/polymer/v3_0/polymer/polymer_bundled.min.js';

import 'chrome://resources/ash/common/cr_elements/md_select.css.js';
import 'chrome://resources/ash/common/cr_elements/cr_shared_style.css.js';
import 'chrome://resources/ash/common/cr_elements/cr_shared_vars.css.js';
import '//resources/polymer/v3_0/paper-progress/paper-progress.js';

import '../../components/buttons/oobe_back_button.js';
import '../../components/buttons/oobe_text_button.js';
import '../../components/buttons/oobe_next_button.js';
import '../../components/common_styles/oobe_common_styles.css.js';
import '../../components/common_styles/cr_card_radio_group_styles.css.js';
import '../../components/common_styles/oobe_dialog_host_styles.css.js';
import '../../components/dialogs/oobe_adaptive_dialog.js';
import '../../components/throbber_notice.js';
import '../../components/gaia_button.js';
import '../../components/notification_card.js';

import {LoginScreenMixin} from '../../components/mixins/login_screen_mixin.js';
import {OobeI18nMixin} from '../../components/mixins/oobe_i18n_mixin.js';
import {OobeDialogHostMixin} from '../../components/mixins/oobe_dialog_host_mixin.js';
import {MultiStepMixin} from '../../components/mixins/multi_step_mixin.js';

import {getSelectedTitle, getSelectedValue, SelectListType, setupSelect} from '../../components/oobe_select.js';

import {getTemplate} from './fydeos_installer.html.js';

class FydeOSInstaller {
  constructor(element: HTMLElement) {
    this.IS_DEBUG = false;
    this.element_ = element;
    this.shellClientKey = '';
    this.forceClosed = false;
    this.asyncTaskCreated = this.asyncTaskCreated.bind(this);
  }

  asyncTaskCreated(key) {
    this.shellClientKey = key;
    console.log('asynctaskcreated, this.shellClientKey', this.shellClientKey);
  }

  forceCloseInstall() {
    return new Promise((resolve) => {
      this.forceClosed = true;
      if (this.shellClientKey) {
        console.log('force close task', this.shellClientKey);
        chrome.shellClient.forceCloseTask(this.shellClientKey, (obj) => {
          console.log('forceCloseTask result', obj);
          resolve(obj);
        });
      } else {
        resolve(null);
      }
    });
  }

  async IsBootingFromRemovable() {
    let ret = await this.execForResult('/usr/sbin/is_running_from_installer');
    ret = ret.trim();
    return ret === 'yes';
  }

  static get SkipRemovableBoards() {
    return [
      'rpi4-fydeos',
    ];
  }

  static get ForceEnableForBoards() {
    return [
      'amd64-generic',
    ];
  }

  static get EFI_TYPE() {
    return 'c12a7328-f81f-11d2-ba4b-00a0c93ec93b';
  }

  static GetDevPath(name) {
    return `/dev/${name}`;
  }

  static SizeInBytes(str) {
    const m = {
      B: 1,
      K: 1024,
      M: 1024 * 1024,
      G: 1024 * 1024 * 1024,
    };
    const unit = str.substr(-1);
    const digit = parseInt(str.substring(0, str.length - 1), 10);
    return digit * m[unit];
  }

  async GetBlockDevList() {
    // TODO real dev
    let blkList = await this.execForResult(
      'lsblk -l -J -o name,size,type,vendor,rev,ro,parttype,rm,mountpoint',
    );
    const mock = `{
"blockdevices": [
  {"name": "loop0", "size": "1.2G", "type": "loop", "vendor": null, "rev": null, "ro": "0", "parttype": null, "rm": "0", "mountpoint": null},
  {"name": "loop1", "size": "90.8M", "type": "loop", "vendor": null, "rev": null, "ro": "1", "parttype": null, "rm": "0", "mountpoint": "/usr/share/chromeos-assets/speech_synthesis/patts"},
  {"name": "sdb", "size": "128.3G", "type": "disk", "vendor": "QEMU    ", "rev": "2.5+", "ro": "0", "parttype": null, "rm": "0", "mountpoint": null},
  {"name": "sdb2", "size": "18G", "type": "part", "vendor": null, "rev": null, "ro": "0", "parttype": "3cb8e202-3b7e-47dd-8a3c-7ff2a13cfcec", "rm": "0", "mountpoint": "/"},
  {"name": "sdb3", "size": "28G", "type": "part", "vendor": null, "rev": null, "ro": "0", "parttype": "3cb8e202-3b7e-47dd-8a3c-7ff2a13cfcec", "rm": "0", "mountpoint": "/"},
  {"name": "sda", "size": "8.3G", "type": "disk", "vendor": "QEMU    ", "rev": "2.5+", "ro": "0", "parttype": null, "rm": "0", "mountpoint": null},
  {"name": "sda1", "size": "4G", "type": "part", "vendor": null, "rev": null, "ro": "0", "parttype": "0fc63daf-8483-4772-8e79-3d69d8477de4", "rm": "0", "mountpoint": "/mnt/stateful_partition"},
  {"name": "sda2", "size": "64M", "type": "part", "vendor": null, "rev": null, "ro": "0", "parttype": "fe3a2a5d-4f32-41a7-b725-accc3285a309", "rm": "0", "mountpoint": null},
  {"name": "sda3", "size": "2G", "type": "part", "vendor": null, "rev": null, "ro": "0", "parttype": "3cb8e202-3b7e-47dd-8a3c-7ff2a13cfcec", "rm": "0", "mountpoint": "/"},
  {"name": "sda4", "size": "64M", "type": "part", "vendor": null, "rev": null, "ro": "0", "parttype": "fe3a2a5d-4f32-41a7-b725-accc3285a309", "rm": "0", "mountpoint": null},
  {"name": "sda5", "size": "2G", "type": "part", "vendor": null, "rev": null, "ro": "0", "parttype": "3cb8e202-3b7e-47dd-8a3c-7ff2a13cfcec", "rm": "0", "mountpoint": null},
  {"name": "sda6", "size": "512B", "type": "part", "vendor": null, "rev": null, "ro": "0", "parttype": "fe3a2a5d-4f32-41a7-b725-accc3285a309", "rm": "0", "mountpoint": null},
  {"name": "sda7", "size": "512B", "type": "part", "vendor": null, "rev": null, "ro": "0", "parttype": "3cb8e202-3b7e-47dd-8a3c-7ff2a13cfcec", "rm": "0", "mountpoint": null},
  {"name": "sda8", "size": "16M", "type": "part", "vendor": null, "rev": null, "ro": "0", "parttype": "0fc63daf-8483-4772-8e79-3d69d8477de4", "rm": "0", "mountpoint": "/usr/share/oem"},
  {"name": "sda9", "size": "512B", "type": "part", "vendor": null, "rev": null, "ro": "0", "parttype": "2e0a753d-9e48-43b0-8337-b15192cb1b5e", "rm": "0", "mountpoint": null},
  {"name": "sda10", "size": "512B", "type": "part", "vendor": null, "rev": null, "ro": "0", "parttype": "2e0a753d-9e48-43b0-8337-b15192cb1b5e", "rm": "0", "mountpoint": null},
  {"name": "sda11", "size": "8M", "type": "part", "vendor": null, "rev": null, "ro": "0", "parttype": "cab6e88e-abf3-4102-a07a-d4bb9be3c1d3", "rm": "0", "mountpoint": null},
  {"name": "sdb12", "size": "128M", "type": "part", "vendor": null, "rev": null, "ro": "0", "parttype": "c12a7328-f81f-11d2-ba4b-00a0c93ec93b", "rm": "0", "mountpoint": null},
  {"name": "sda22", "size": "90G", "type": "part", "vendor": null, "rev": null, "ro": "0", "parttype": "x12a7328-f81f-11d2-ba4b-00a0c93ec93b", "rm": "0", "mountpoint": "/tmp/fydeos_dualboot_XXX"},
  {"name": "zram0", "size": "11.4G", "type": "disk", "vendor": null, "rev": null, "ro": "0", "parttype": null, "rm": "0", "mountpoint": "[SWAP]"},
  {"name": "encstateful", "size": "1.2G", "type": "dm", "vendor": null, "rev": null, "ro": "0", "parttype": null, "rm": "0", "mountpoint": "/mnt/stateful_partition/encrypted"},
  {"name": "sdc", "size": "128.3G", "type": "disk", "vendor": "QEMU    ", "rev": "2.5+", "ro": "0", "parttype": null, "rm": "0", "mountpoint": null},
  {"name": "sdc1", "size": "9G", "type": "part", "vendor": null, "rev": null, "ro": "0", "parttype": "3cb8e202-3b7e-47dd-8a3c-7ff2a13cfcec", "rm": "0", "mountpoint": "/"},
  {"name": "sdc2", "size": "29G", "type": "part", "vendor": null, "rev": null, "ro": "0", "parttype": "3cb8e202-3b7e-47dd-8a3c-7ff2a13cfcec", "rm": "0", "mountpoint": "/"},
  {"name": "sdc12", "size": "128M", "type": "part", "vendor": null, "rev": null, "ro": "0", "parttype": "c12a7328-f81f-11d2-ba4b-00a0c93ec93b", "rm": "0", "mountpoint": null}
]
}`;
    if (this.IS_DEBUG) blkList = mock;
    try {
      const arr = JSON.parse(blkList).blockdevices;
      return arr;
    } catch (err) {
      return [];
    }
  }

  async GetRootDevPath() {
    const ret = await this.execForResult('rootdev -s');
    return ret ? ret.trim() : '';
  }

  static get InstallCommand() {
    return '/usr/sbin/chromeos-install';
  }

  async Install({ diskPath }) {
    console.log('install diskpath', diskPath);
    const self = this;
    self.forceClosed = false;
    const processParser = [
      { filter: 'Installing partition 5', indicator: 20 },
      { filter: 'Installing partition 3', indicator: 40 },
      { filter: 'Installing the stateful partition', indicator: 60 },
    ];
    let indicator = 0;

    function dispatchSuccess() {
      self.dispatchEvent(FydeOSInstaller.Events.OnSuccess);
    }

    function dispatchError(error) {
      if (self.forceClosed) return;
      self.dispatchEvent(FydeOSInstaller.Events.OnError, { detail: { error }});
    }

    function findProgressIndicator(result, filter) {
      try {
        const lines = result.split('\n');
        for (let i = 0; i < lines.length; i += 1) {
          const line = lines[i];
          if (line.startsWith(filter)) {
            return true;
          }
        }
        return false;
      } catch (err) {
        return false;
      }
    }

    function dispatchProcess(result) {
      // debug(result);
      let findIndicator = false;
      for (let i = indicator; i < processParser.length; i += 1) {
        if (findProgressIndicator(result,processParser[i].filter)) {
          self.dispatchEvent(FydeOSInstaller.Events.OnProgress, { detail: { desc: processParser[i].filter, indicator: processParser[i].indicator } });
          indicator = i + 1;
          findIndicator = true;
          break;
        }
      }
      if (!findIndicator) {
        self.dispatchEvent(FydeOSInstaller.Events.OnProgress, { detail: { desc: result ? result.trim() : '' } });
      }
    }

    return this.isARM().then((ret) => {
      let extraArray = []
      if (ret) {
        extraArray.push('--skip_postinstall');
      }
      const board = loadTimeData.getString('lsbReleaseBoard');
      if (FydeOSInstaller.SkipRemovableBoards.indexOf(board) !== -1) {
        extraArray.push('--skip_src_removable');
        extraArray.push('--skip_dst_removable');
      }
      const extra = extraArray.join(' ');
      return this.execForLongTask(`${FydeOSInstaller.InstallCommand} ${extra} --dst ${diskPath} --yes`,
        this.asyncTaskCreated,
        dispatchSuccess, dispatchError, 999999, 2, 10, dispatchProcess);
    });
  }

  static get Events() {
    return {
      OnProgress: 'on_install_progress',
      OnError: 'on_install_error',
      OnSuccess: 'on_install_success',
    };
  }

  async WaitForShellClient() {
    const testCmd = 'true';
    for (let i = 0; i < 7; i++) {
      try {
        await this.execForResult(testCmd);
        return true;
      } catch (err) {
        console.log(err);
        await this.sleep(2);
        continue;
      }
    }

    return false;
  }

  async IsDualbootInstallEnabled() {
    if (this.IS_DEBUG) return true;
    const ret = await this.isFileExist('/usr/share/dualboot/install_fydeos_loader.sh');
    return ret;
  }

  Shutdown() {
    return this.execForResult('powerd_setuid_helper --action=shut_down');
  }

  async execForResult(cmd) {
    if (this.IS_DEBUG) return '';
    return new Promise((resolve, reject) => {
      console.log('shellClient execSync cmd', cmd)
      chrome.shellClient.execSync(cmd, (ret) => {
        console.log('shellClient execSync result', ret);
        const { result, code } = ret;
        if (code !== 0) {
          reject(new Error(result));
        } else {
          resolve(result);
        }
      });
    })
  }

  async isFileExist(file) {
    try {
      await this.execForResult(`ls ${file}`);
      return true;
    } catch (err) {
      return false;
    }
  }

  async sleep(sec) {
    return new Promise(resolve => {
      setTimeout(() => {
        resolve();
      }, sec * 1000);
    });
  }

  dispatchEvent(type, custom) {
    if (!this.element_) return;
    this.element_.dispatchEvent(new CustomEvent(type, custom));
  }

  async isARM() {
    const arch = await this.getArch();
    if (!arch) return false;
    const { machine, processor } = arch;
    try {
      return (machine.indexOf('arm') !== -1) || (machine.indexOf('aarch64') !== -1) || (processor.indexOf('arm') !== -1);
    } catch (err) {
      return false;
    }
  }

  async getArch() {
    try {
      const machine = await this.execForResult('uname -m');
      const processor = await this.execForResult('uname -p')
      return {
        machine: machine.trim().toLowerCase(),
        processor: processor.trim().toLowerCase(),
      };
    } catch (err) {
      console.error('get architecture failed', err);
      return null;
    }
  }

  async clearDualbootLog() {
    try {
      await this.execForResult(FydeOSInstaller.UtilWrapperCMD('clear_log'));
    } catch (err) {
      console.log(err);
    }
  }

  static get DualbootCommand() {
    return '/usr/sbin/dual-boot-install';
  }

  static InstallrEFIndCMD(devPath) {
    return `/usr/share/dualboot/install_refind.sh -d ${devPath}`;
  }

  async getFileContent(file) {
    if (!file) return '';
    let ret = '';
    try {
      ret = await this.execForResult(`cat ${file}`);
    } catch (err) {
      ret = '';
    }
    return ret;
  }

  async GetDualBootInstallLog() {
    const ret = await this.getFileContent('/tmp/fydeos_dualboot.log');
    return ret;
  }

  async GetDualBootUnInstallLog() {
    const ret = await this.getFileContent('cat /tmp/uninstall_dualboot.log');
    return ret;
  }

  async isEnableUninstall() {
    const script = '/usr/share/dualboot/uninstall_all.sh';
    let enabled = false;
    try {
      const ret = await this.isFileExist(script);
      enabled = !!ret;
    } catch (err) {
      enabled = false;
    }
    return enabled;
  }

  async uninstallDualboot() {
    const script = '/usr/share/dualboot/uninstall_all.sh';
    const enabled = await this.isEnableUninstall();
    if (!enabled) {
      return null;
    }
    return new Promise((resolve, reject) => {
      this.execForLongTask(script, this.asyncTaskCreated, (result) => {
        resolve(result);
      }, (err) => {
        reject(err);
      }, 600, 2, 10, null);
    });
  }

  static UtilWrapperCMD(cmd, args) {
    let strArg;
    if (typeof args === 'string') {
      strArg = args;
    } else if (Array.isArray(args)) {
      strArg = args.join(' ');
    } else {
      strArg = '';
    }
    return `/usr/share/dualboot/fydeos_util_wrapper.sh ${cmd} ${strArg}`;
  }

  static InstallKernelCMD(devPath) {
    return `/usr/share/dualboot/install_kernel.sh -d ${devPath}`;
  }

  static InstallImageCMD(devPath) {
    return FydeOSInstaller.UtilWrapperCMD('create_dualboot_image', devPath);
  }

  static InstallLoaderCMD(devPath) {
    return `/usr/share/dualboot/install_fydeos_loader.sh -d ${devPath}`;
  }

  DualbootInstall({ partPath, efiPath, installrEFI = true, installBoot = false }) {
    console.log(`multi-boot install, partPath: ${partPath}, efiPath: ${efiPath}, installrEFI: ${installrEFI}`);
    const self = this;

    this.forceClosed = false;

    function dispatchError(desc, err) {
      if (self.forceClosed) return;
      self.dispatchEvent(FydeOSInstaller.Events.OnError, { detail: { error: err, desc } });
    }

    let indicator = 0;
    const processParser = [
      { filter: 'Installing partition 5', indicator: 40 },
      { filter: 'Installing partition 3', indicator: 50 },
      { filter: 'Installing the stateful partition', indicator: 60 },
    ];

    function dispatchProcess(result) {
      // debug(result);
      for (let i = indicator; i < processParser.length; i += 1) {
        if (result.startsWith(processParser[i].filter)) {
          self.dispatchEvent(FydeOSInstaller.Events.OnProgress, { detail: { desc: processParser[i].filter, indicator: processParser[i].indicator  } });
          indicator = i + 1;
          break;
        }
      }
    }

    function dispatchSuccess() {
      self.dispatchEvent(FydeOSInstaller.Events.OnSuccess);
    }

    const process = [
      {
        command: FydeOSInstaller.UtilWrapperCMD('safe_format', partPath),
        enable: true,
        descript: 'Format partition for installation',
        timeout: 300,
        onProgress: null,
        indicator: 20,
      },
      {
        command: FydeOSInstaller.InstallKernelCMD(partPath),
        enable: true,
        descript: 'Install dual boot kernel and dependencies',
        timeout: 10,
        onProgress: null,
        indicator: 30,
      },
      {
        command: FydeOSInstaller.InstallImageCMD(partPath),
        enable: true,
        descript: 'Create dual boot image',
        timeout: 60 * 10,
        onProgress: dispatchProcess,
        indicator: 70,
      },
      {
        command: FydeOSInstaller.InstallLoaderCMD(efiPath),
        enable: true,
        descript: 'Install dual boot loader to EFI partition',
        timeout: 10,
        onProgress: null,
        indicator: 80,
      },
      {
        command: FydeOSInstaller.InstallrEFIndCMD(efiPath),
        enable: installrEFI,
        descript: 'Install rEFInd to EFI partition',
        timeout: 20,
        onProgress: null,
        indicator: 90,
      },
      {
        // <if expr="openfyde">
        command: FydeOSInstaller.UtilWrapperCMD('safe_create_entry', ['/EFI/openfyde/bootx64.efi', 'openFyde_Dualboot_Loader', efiPath]),
        // </if>
        // <if expr="not openfyde">
        command: FydeOSInstaller.UtilWrapperCMD('safe_create_entry', ['/EFI/fydeos/bootx64.efi', 'FydeOS_Dualboot_Loader', efiPath]),
        // </if>
        enable: installBoot,
        descript: 'Install boot entry to bios',
        timeout: 5,
        onProgress: null,
        indicator: 90,
      },
    ];

    function Exector() {
      if (self.forceClosed) return;
      if (process.length === 0) {
        dispatchSuccess('All mission accomplish.');
        return;
      }
      const mission = process.shift();
      if (mission.enable) {
        self.execForLongTask(mission.command, self.asyncTaskCreated, () => {
          self.dispatchEvent(FydeOSInstaller.Events.OnProgress, { detail: { desc: mission.descript, indicator: mission.indicator } });
          Exector();
        }, async (err) => {
          const errorLog = await self.GetDualBootInstallLog();
          dispatchError(mission.descript, errorLog || err);
        }, 999999, 2, 1, mission.onProgress);
      } else {
        Exector();
      }
    }

    return self.clearDualbootLog().then(() => {
      Exector();
    });
  }

  execForLongTask(
    cmd,
    createdCB,
    successCB,
    faildCB,
    timeout = 300,
    interval = 1,
    outputLines = 10,
    intervalCB = null,
  ) {
    console.log('shellClient execAsync cmd', cmd);
    chrome.shellClient.execAsync(cmd, (obj) => {
      if (obj.code === -1) return faildCB(new Error(obj.result));
      const key = obj.code;
      createdCB(key);
      let counter = Math.round(timeout / interval);
      function checkOutput() {
        counter--;
        if (counter >= 0) {
          chrome.shellClient.getTaskOutput(key, outputLines, (obj) => {
            switch (obj.code) {
              case chrome.shellClient.ON_PROCESS:
                if (intervalCB) intervalCB(obj.result);
                break;
              case chrome.shellClient.ON_CLOSED:
                return successCB(obj.result);
              case chrome.shellClient.ON_ERROR:
                return faildCB(new Error(obj.result));
              case chrome.shellClient.ON_NONE:
                return faildCB(new Error('The task has been terminated.'));
              default:
                return faildCB(new Error('Task may be terminated by others'));
            }
            setTimeout(checkOutput, interval * 1000);
          });
        } else {
          chrome.shellClient.forceCloseTask(key, (obj) => {
            faildCB(new Error('The mission is force closed for timeout!'));
          });
        }
      }
      checkOutput();
    });
  }
}

// @ts-check

// minimal 10G
// recommended 20G
const minimumSize = 10 * 1024 * 1024 * 1024;
const recommandedSize = 20 * 1024 * 1024 * 1024;

const availOSPart = dev => {
  const okSize = FydeOSInstaller.SizeInBytes(dev.size) > minimumSize;
  const notEfi = dev.parttype !== FydeOSInstaller.EFI_TYPE;
  return okSize && notEfi;
}

const availEFIPart = dev => {
  const okSize = FydeOSInstaller.SizeInBytes(dev.size) > 0;
  const notRm = dev.rm !== '1';
  const notRo = dev.ro !== '1';
  const isEfi = dev.parttype === FydeOSInstaller.EFI_TYPE;
  return okSize && notRm && notRo && isEfi;
}

const candidateOSPart = dev => {
  return availOSPart(dev) && FydeOSInstaller.SizeInBytes(dev.size) > recommandedSize && !(dev.mountpoint && dev.mountpoint.startsWith('/tmp/fydeos_dualboot_'));
}

enum InstallSteps {
  PREPARE = 'prepare',
  PORTAL = 'portal',
  NORMAL_CONFIG = 'normal-config',
  DUALBOOT_CONFIG = 'dualboot-config',
  INSTALLING = 'installing',
  ABORTING = 'aborting',
  DONE = 'done',
};

enum InstallTypes {
  NORMAL = 'normal',
  DUALBOOT = 'dualboot',
};

enum InstallResults {
  UNKNOWN = 'unknown',
  SUCCESS = 'success',
  FAILURE = 'failure',
  ABORT = 'abort',
};

enum MultiBootOptions {
  NONE = 'none',
  REFIND = 'refind',
  UEFI = 'uefi',
};

const DiskToSelectOption = disk => {
  let title = FydeOSInstaller.GetDevPath(disk.name);
  if (disk.size) {
    title = `${FydeOSInstaller.GetDevPath(disk.name)} (${disk.size})`;
  }
  const value = disk.name;
  return { title, value };
};

const FydeOSInstallerScreenElementBase = OobeDialogHostMixin(LoginScreenMixin(MultiStepMixin(OobeI18nMixin(PolymerElement))));

/**
 * @polymer
 */
class FydeOSInstallerScreen extends FydeOSInstallerScreenElementBase {
  static get is() {
    return 'oobe-fydeos-installer';
  }

  get EXTERNAL_API() {
    return [
      'setIsBrandedBuild',
    ];
  } 

  static get template() {
    return getTemplate();
  }


  static get properties() {
    return {
      isLoading_: {
        type: Boolean,
        value: false,
      },
      isShellClientError_: {
        type: Boolean,
        value: false,
      },
      isShellClientAvailable_: {
        type: Boolean,
        value: false,
      },
      isDualbootInstallEnabled_: {
        type: Boolean,
        value: true,
      },
      selectedInstallType_: {
        type: String,
        value: InstallTypes.NORMAL,
      },
      normalConfigSelectedDisk_: {
        type: String,
        value: '',
      },
      normalConfigMessage_: {
        type: String,
        value: '',
      },
      dualbootConfigSelectEFIMessage_: {
        type: String,
        value: '',
      },
      dualbootConfigSelectOSPartMessage_: {
        type: String,
        value: '',
      },
      dualbootConfigSelectedOSPart_: {
        type: String,
        value: '',
      },
      dualbootConfigSelectedEFIPart_: {
        type: String,
        value: '',
      },
      isNormalSetup_: {
        type: Boolean,
        value: false,
        computed: 'computeIsNormalSetup_(normalConfigSelectedDisk_)',
      },
      isDualbootSetup_: {
        type: Boolean,
        value: false,
        computed: 'computeIsDualbootSetup_(dualbootConfigSelectedOSPart_, dualbootConfigSelectedEFIPart_)',
      },
      installingTitle_: {
        type: String,
        value: '',
        computed: 'computeInstallingTitle_(uiStep, result_)',
      },
      installOutput_: {
        type: String,
        value: '',
      },
      trimedInstallOutput_: {
        type: String,
        value: '',
        computed: 'computeTrimedInstallOutput_(installOutput_)',
        observer: 'handleTrimedInstallOutputChanged_',
      },
      progressMessage_: {
        type: String,
        value: '',
      },
      progressValue_: {
        type: Number,
        value: 0,
      },
      result_: {
        type: String,
        value: InstallResults.UNKNOWN,
      },
      multiBootOptionSelected_: {
        type: String,
        value: '',
        observer: 'onMultiBootOptionSelectedChanged_',
      },
      multiBootOptionDesc_: {
        type: String,
        value: '',
      },
    };
  }

  private isLoading_: boolean;
  private isShellClientError_: boolean;
  private isShellClientAvailable_: boolean;
  private isDualbootInstallEnabled_: boolean;
  private selectedInstallType_: InstallTypes;;
  private normalConfigSelectedDisk_: string;
  private normalConfigMessage_: string;
  private dualbootConfigSelectEFIMessage_: string;
  private dualbootConfigSelectOSPartMessage_: string;
  private dualbootConfigSelectedOSPart_: string;
  private dualbootConfigSelectedEFIPart_: string;
  private isNormalSetup_: boolean;
  private isDualbootSetup_: boolean;
  private installingTitle_: string;
  private installOutput_: string;
  private trimedInstallOutput_: string;
  private progressMessage_: string;
  private progressValue_: number;
  private result_: InstallResults;
  private multiBootOptionSelected_: string;
  private multiBootOptionDesc_: string;

  get UI_STEPS() {
    return InstallSteps;
  }

  constructor() {
    super();
    this.fullDiskList_ = [];
    this.rootDev_ = '';
  }

  override defaultUIStep(): InstallSteps {
    return InstallSteps.PREPARE;
  }

  override ready() {
    super.ready();
    this.initializeLoginScreen('OsInstallScreen', {
      resetAllowed: true,
    });

    this.installer_ = new FydeOSInstaller(this);

    this.addEventListener(FydeOSInstaller.Events.OnProgress, this.handleInstallProgess_.bind(this));
    this.addEventListener(FydeOSInstaller.Events.OnSuccess, this.handleInstallSuccess_.bind(this));
    this.addEventListener(FydeOSInstaller.Events.OnError, this.handleInstallError_.bind(this));

    this.$['shellclient-error'].addEventListener('buttonclick', function () {
      this.handleShellClientErrorButtonClick_();
    }.bind(this));

    this.waitForShellClient_();
  }

  setupMultiBootOptionsSelect_() {
    this.multiBootOptions_ = [{
      title: this.i18n('fydeosInstallerInstallREFIndTitle'),
      value: MultiBootOptions.REFIND,
    }, {
      title: this.i18n('fydeosInstallerInstallUefiBootTitle'),
      value: MultiBootOptions.UEFI,
    }, {
      title: this.i18n('fydeosInstallerMultiBootConfigureByYourselfTitle'),
      value: MultiBootOptions.NONE,
    }];

    this.setupSelect_(this.$.multiBootOptionList, this.multiBootOptions_, this.onMultiBootOptionSelected_.bind(this));
  }

  async waitForShellClient_() {
    this.isLoading_ = true;
    const ret = await this.installer_.WaitForShellClient();
    let isBootingFromRemovable = false;
    if (ret) {
      this.isShellClientAvailable_ = true;
      this.isDualbootInstallEnabled_ = await this.installer_.IsDualbootInstallEnabled();
      this.rootDev_ = await this.installer_.GetRootDevPath();
      this.fullDiskList_ = await this.installer_.GetBlockDevList();
      if (!this.rootDev_ || !this.fullDiskList_.length) {
        this.isShellClientError_ = true;
      } else {
        this.onPrepared_();
      }
      isBootingFromRemovable = await this.installer_.IsBootingFromRemovable();
    } else {
      console.log('shellClient does not work');
      this.isShellClientError_ = true;
    }
    const board = loadTimeData.getString('lsbReleaseBoard');
    console.log('isBootingFromRemovable', isBootingFromRemovable);
    console.log('board', board);
    const forceShow = this.IS_DEBUG || (FydeOSInstaller.ForceEnableForBoards.indexOf(board) !== -1);
    let visible = this.IS_DEBUG || !this.isShellClientError_;
    visible = visible && (isBootingFromRemovable || forceShow);

    this.isLoading_ = false;
  }

  async refreshDevList_() {
    this.fullDiskList_ = await this.installer_.GetBlockDevList();
    if (!this.fullDiskList_.length) {
      this.isShellClientError_ = true;
      return false;
    }
    return true;
  }

  handleShellClientErrorButtonClick_() {
    this.close_();
  }

  resetStates_() {
    this.normalConfigSelectedDisk_ = '';
    this.normalConfigMessage_ = '';
    this.dualbootConfigSelectOSPartMessage_ = '';
    this.dualbootConfigSelectEFIMessage_ = '';
    this.dualbootConfigSelectedOSPart_ = '';
    this.dualbootConfigSelectedEFIPart_ = '';
    this.multiBootOptionSelected_ = '';
    this.isNormalSetup_ = false;
    this.isDualbootSetup_ = false;
    this.installingTitle_ = this.i18n('fydeosInstallerInstalling');
    this.progressValue_ = 0;
    this.progressMessage_ = '';
    this.installOutput_ = '';
    this.result_ = InstallResults.UNKNOWN;
  }

  reloadAll_() {
    this.resetStates_();
    this.setUIStep(InstallSteps.PREPARE);
    this.waitForShellClient_();
  }

  shouldShowShellClientError_() {
    return !this.isLoading_ && this.isShellClientError_;
  }

  onBeforeShow() {
    super.onBeforeShow();
    this.setupMultiBootOptionsSelect_();
    if (this.isLoading_) return;
    if (!this.isShellClientAvailable_) {
      this.reloadAll_();
    } else {
      this.showAgain_();
    }
  }

  show() {
    console.log('fydeos installer show');
  }

  async showAgain_() {
    if (this.uiStep === InstallSteps.INSTALLING) return;
    const ret = await this.refreshDevList_();
    if (ret) {
      this.onPrepared_();
    }
  }

  onPrepared_() {
    this.resetStates_();
    this.setUIStep(InstallSteps.PORTAL);
  }

  handleInstallProgess_(e) {
    if (this.uiStep === InstallSteps.DONE) return;
    const { detail } = e;
    console.log('handleInstallProgess_', detail);
    const { indicator, desc } = detail;
    const number = parseInt(indicator, 10);
    if (!Number.isNaN(number)) {
      this.progressValue_ = number;
    }
    if (desc) {
      this.installOutput_ = desc.trim();
    }
  }

  handleInstallSuccess_() {
    if (this.uiStep === InstallSteps.DONE) return;
    this.progressValue_ = 100;
    this.setUIStep(InstallSteps.DONE);
    this.result_ = InstallResults.SUCCESS;
    this.installOutput_ = '';
    this.progressMessage_ = this.i18n('fydeosInstallerInstallSuccessMessage');
  }

  handleInstallError_(e) {
    if (this.uiStep === InstallSteps.DONE) return;
    const { detail: { error } } = e;
    console.log('handleInstallError_', error);
    this.setUIStep(InstallSteps.DONE);
    this.result_ = InstallResults.FAILURE;
    let output = ''
    if (error) {
      if (typeof error === 'string') {
        output = error;
      } else if (error.message) {
        output  = error.message;
      }
    }
    if (output) {
      this.installOutput_ = output.trim();
    }
    this.progressMessage_ = this.i18n('fydeosInstallerInstallFailedMessage');
  }

  close_() {
    if (!this.uiStep === InstallSteps.INSTALLING) return;
    this.userActed('os-install-exit');
  }

  onConfigClicked_() {
    if (this.selectedInstallType_ === InstallTypes.NORMAL) {
      this.setUIStep(InstallSteps.NORMAL_CONFIG);
      this.prepareConfigForNormalInstall_();
    } else if (this.selectedInstallType_ === InstallTypes.DUALBOOT) {
      this.setUIStep(InstallSteps.DUALBOOT_CONFIG);
      this.prepareConfigForDualbootInstall_();
    }
  }

  isRootDev_(disk) {
    if (this.IS_DEBUG) return false;
    return this.rootDev_.startsWith(FydeOSInstaller.GetDevPath(disk.name));
  }

  setupSelect_(element, list, callback) {
    if (!element) return;
    // const defaultValue = '___default___';
    // const selectList = [{ title: '', value: defaultValue }, ...list];
    setupSelect(element, list, (value) => {
      // if (value !== defaultValue) {
      callback(value);
      // }
    });
    element.value = null;
  }

  prepareConfigForNormalInstall_() {
    if (this.isNormalSetup_) return;
    const list = this.fullDiskList_.filter(d => {
      const available = d.type === 'disk' && d.name !== 'zram0' && !this.isRootDev_(d);
      const enoughDiskSize = (!d.size) || (d.size && FydeOSInstaller.SizeInBytes(d.size) > minimumSize);
      return available && enoughDiskSize;
    });
    if (!list.length) {
      this.normalConfigMessage_ = this.i18n('fydeosInstallerNormalConfigNoAvailableDisk');
      return;
    }

    const selectList = list.map(d => DiskToSelectOption(d));
    if (selectList.length) {
      this.setupSelect_(this.$.normalConfigDiskList, selectList, this.onNormalConfigSelectDiskSelected_.bind(this));
    }

    if (selectList.length === 1) {
      this.normalConfigSelectedDisk_ = selectList[0].value;
    }
  }

  onNormalConfigSelectDiskSelected_(value) {
    this.normalConfigSelectedDisk_ = value;
  }

  computeIsNormalSetup_(normalConfigSelectDisk) {
    return !!normalConfigSelectDisk;
  }

  computeIsDualbootSetup_(dualbootConfigSelectedOSPart, dualbootConfigSelectedEFIPart) {
    return dualbootConfigSelectedOSPart && dualbootConfigSelectedOSPart.length && dualbootConfigSelectedEFIPart && dualbootConfigSelectedEFIPart.length;
  }

  prepareConfigForDualbootInstall_() {
    if (this.isDualbootSetup_) return;
    const osPartList = [];
    const efiPartList = [];
    const availableDiskList = this.fullDiskList_.filter(d => (d.type === 'disk' && d.name !== 'zram0' && !this.isRootDev_(d)));

    for (let i = 0; i < availableDiskList.length; i++) {
      const disk = availableDiskList[i];
      osPartList.push({ optionGroupName: FydeOSInstaller.GetDevPath(disk.name) });
      efiPartList.push({ optionGroupName: FydeOSInstaller.GetDevPath(disk.name) });
      const availOSPartList = this.fullDiskList_.filter(p => p.type === 'part' && availOSPart(p) && p.name.startsWith(disk.name));
      for (let j = 0; j < availOSPartList.length; j++) {
        osPartList.push(DiskToSelectOption(availOSPartList[j]));
      }
      const availEFIPartList = this.fullDiskList_.filter(p => p.type === 'part' && p.name.startsWith(disk.name) && availEFIPart(p));
      for (let j = 0; j < availEFIPartList.length; j++) {
        efiPartList.push(DiskToSelectOption(availEFIPartList[j]));
      }
    }
    if (osPartList.length) {
      this.setupSelect_(this.$.dualbootConfigOSPartList, osPartList, this.onDualbootConfigOSPartSelected_.bind(this));
    }
    if (efiPartList.length) {
      this.setupSelect_(this.$.dualbootConfigEFIPartList, efiPartList, this.onDualbootConfigEFIPartSelected_.bind(this));
    }

    const candidateOSPartList = this.fullDiskList_.filter(p => p.type === 'part' && candidateOSPart(p));
    const candidateEFIPartList = this.fullDiskList_.filter(p => p.type === 'part' && availEFIPart(p));
    if (candidateOSPartList.length === 1) {
      this.handleDualbootConfigOSPartChange_(candidateOSPartList[0].name);
    }
    if (candidateEFIPartList.length === 1) {
      this.handleDualbootConfigEFIPartChange_(candidateEFIPartList[0].name);
    }
  }

  onDualbootConfigOSPartSelected_(value) {
    this.handleDualbootConfigOSPartChange_(value);
  }

  onDualbootConfigEFIPartSelected_(value) {
    this.handleDualbootConfigEFIPartChange_(value);
  }

  handleDualbootConfigOSPartChange_(partName) {
    this.dualbootConfigSelectedOSPart_ = partName;
    if (!partName) return;
    console.log('handleDualbootConfigOSPartChange_', partName);
    const dev = this.fullDiskList_.find(d => d.name === partName);
    if (!dev) {
      this.dualbootConfigSelectedOSPart_ = null;
      return;
    }
    if (dev.mountpoint && dev.mountpoint.startsWith('/tmp/fydeos_dualboot_')) {
      this.dualbootConfigSelectOSPartMessage_ = this.i18n('fydeosInstallerDualbootSelectedOSPartAlreadyInstalled');
    } else if (FydeOSInstaller.SizeInBytes(dev.size) < recommandedSize) {
      this.dualbootConfigSelectOSPartMessage_ = this.i18n('fydeosInstallerDualbootSelectedOSPartSizeSmaller');
    } else {
      this.dualbootConfigSelectOSPartMessage_ = '';
    }
  }

  handleDualbootConfigEFIPartChange_(partName) {
    console.log('handleDualbootConfigEFIPartChange_', partName);
    this.dualbootConfigSelectedEFIPart_ = partName;
    if (!partName) return;
    const dev = this.fullDiskList_.find(d => d.name === partName);
    if (!dev) {
      this.dualbootConfigSelectOSPartMessage_ = null;
      return;
    }
  }

  onMultiBootOptionSelected_(value) {
    this.multiBootOptionSelected_ = value;
  }

  onMultiBootOptionSelectedChanged_() {
    switch (this.multiBootOptionSelected_) {
      case MultiBootOptions.REFIND:
        this.multiBootOptionDesc_ = this.i18n('fydeosInstallerInstallREFIndHelperText');
        break;
      case MultiBootOptions.UEFI:
        this.multiBootOptionDesc_ = this.i18n('fydeosInstallerInstallUefiBootHelperText');
        break;
      case MultiBootOptions.NONE:
        this.multiBootOptionDesc_ = this.i18n('fydeosInstallerMultiBootConfigureByYourselfHelperText');
        break;
      default:
        this.multiBootOptionDesc_ = '';
        break;
    }
  }

  onInstallClicked_() {
    this.setUIStep(InstallSteps.INSTALLING);
    this.progressValue_ = 0;
    this.progressMessage_ = '';
    this.installOutput_ = '';
    this.result_ = InstallResults.UNKNOWN;
    if (this.selectedInstallType_ === InstallTypes.NORMAL) {
      this.installer_.Install({ diskPath: FydeOSInstaller.GetDevPath(this.normalConfigSelectedDisk_) });
    } else if (this.selectedInstallType_ === InstallTypes.DUALBOOT) {
      this.installer_.DualbootInstall({
        partPath: FydeOSInstaller.GetDevPath(this.dualbootConfigSelectedOSPart_),
        efiPath: FydeOSInstaller.GetDevPath(this.dualbootConfigSelectedEFIPart_),
        installrEFI: this.multiBootOptionSelected_ === MultiBootOptions.REFIND,
        installBoot: this.multiBootOptionSelected_ === MultiBootOptions.UEFI,
      });
    }
  }

  onShutdownClicked_() {
    this.installer_.Shutdown();
  }

  onBackClicked_() {
    switch (this.uiStep) {
      case InstallSteps.PORTAL:
        this.close_();
        break;
      case InstallSteps.NORMAL_CONFIG:
      case InstallSteps.DUALBOOT_CONFIG:
        this.setUIStep(InstallSteps.PORTAL);
        break;
      case InstallSteps.DONE:
        if (this.result_ === InstallResults.SUCCESS) {
          this.close_();
        } else if (this.result_ === InstallResults.FAILURE || this.result_ === InstallResults.ABORT) {
          if (this.selectedInstallType_ === InstallTypes.NORMAL) {
            this.setUIStep(InstallSteps.NORMAL_CONFIG);
          } else if (this.selectedInstallType_ === InstallTypes.DUALBOOT) {
            this.setUIStep(InstallSteps.DUALBOOT_CONFIG);
          }
        }
        break;
      default:
        break;
    }
  }

  shouldShowShutdownButton_() {
    return this.uiStep === InstallSteps.DONE && this.result_ === InstallResults.SUCCESS;
  }

  shouldEnableBackButton_() {
    return this.uiStep !== InstallSteps.INSTALLING && this.uiStep !== InstallSteps.ABORTING;
  }

  shouldShowBackButton_() {
    return this.uiStep === InstallSteps.DONE && (this.result_ === InstallResults.ABORT || this.result_ === InstallResults.FAILURE);
  }

  computeInstallingTitle_(step, result) {
    if (step === InstallSteps.DONE) {
      if (result === InstallResults.SUCCESS) {
        return this.i18n('fydeosInstallerInstallSuccessTitle');
      } else if (result === InstallResults.FAILURE) {
        return this.i18n('fydeosInstallerInstallFailedTitle');
      } else if (result === InstallResults.ABORT) {
        return this.i18n('fydeosInstallerAbortTitle');
      }
    } else if (step === InstallSteps.ABORTING) {
      return this.i18n('fydeosInstallerAbortTitle');
    } else {
      return this.i18n('fydeosInstallerInstalling');
    }
  }

  progressMessageClass_() {
    if (this.result_ === InstallResults.FAILURE) {
      return 'progress-error progress-message';
    } else {
      return 'progress-message';
    }
  }

  async safeUninstallDualboot_() {
    if (this.selectedInstallType_ !== InstallTypes.DUALBOOT) return;
    const canUninstall = await this.installer_.isEnableUninstall();
    if (!canUninstall) return;
    try {
      await this.installer_.uninstallDualboot();
    } catch (err) {
      console.log('uninstall error', err);
    }
  }

  async onAbortClicked_(e) {
    e.stopPropagation();
    this.setUIStep(InstallSteps.ABORTING);
    this.progressMessage_ = this.i18n('fydeosInstallerAbortingMessage');
    await this.installer_.forceCloseInstall();
    await this.safeUninstallDualboot_();
    this.setUIStep(InstallSteps.DONE);
    this.result_ = InstallResults.ABORT;
    this.progressMessage_ = this.i18n('fydeosInstallerAbortMessage');
  }

  shouldProgressIndeterminate_() {
    return (this.uiStep === InstallSteps.INSTALLING && this.progressValue_ === 0) || this.uiStep === InstallSteps.ABORTING;
  }

  shouldEnableAbortButton_() {
    return this.uiStep !== InstallSteps.ABORTING;
  }

  shouldShowAbortButton_() {
    return this.uiStep === InstallSteps.INSTALLING;
  }

  setIsBrandedBuild(is_branded) {
    // empty
  }

  computeTrimedInstallOutput_(installOutput) {
    return installOutput.replace(/\[.*?\]:.*?:\s?/g, '').replaceAll('\b', '');
  }

  handleTrimedInstallOutputChanged_() {
    const ele = this.$.installShellOutput;
    if (!ele || ele.scrollHeight === undefined) return;
    ele.scrollTop = ele.scrollHeight;
  }
}

customElements.define(FydeOSInstallerScreen.is, FydeOSInstallerScreen);
