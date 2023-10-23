// Copyright 2021 Fyde Innovations. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview Helper functions to popup cashier web page.
 */

import {loadTimeData} from 'chrome://resources/js/load_time_data.js';

export const RenewalStatus = {
  OK: 'ok',
  FAILED: 'failed',
  CHECKING: 'checking',
  SINGLE_UPGRADE: 'single-upgrade',
  LICENSE_EXPIRED: 'license-expired',
};

/** @interface */
export class PopupLicenseWindowProxy {
  init() {}
  popupRenew() {}
  popupUpgrade() {}
}

const CASHIER_SERVER_URL = loadTimeData.getString('fydeosLicenseUrl');

let instance: PopupLicenseWindowProxy|null = null;

/**
 * @implements {PopupLicenseWindowProxy}
 */
export class PopupLicenseWindowProxyImpl {
  private boardName_: string;
  private language_: string;
  private licenseId_: string;

  static getInstance() {
    return instance || (instance = new PopupLicenseWindowProxyImpl());
  }

  async init() {
    this.boardName_ = this.getBoardName_();
    this.language_ = this.getLanguage_();
    this.licenseId_ = await this.getLicenseId_();
  }

  getLanguage_() {
    return loadTimeData.getString('language');
  }

  getBoardName_() {
    return loadTimeData.getString('aboutFydeOSBoardName');
  }

  executeCommand_(command: string) {
    return new Promise<string>((resolve, reject) => {
      chrome.shellClient.execSync(command, (response) => {
        if (!response) reject(new Error('No response'));
        const { code, result } = response;
        if (code !== 0) {
          reject(new Error(result || `Exit code ${code}`));
        } else {
          resolve(result);
        }
      });
    });
  }

  async getLicenseId_() {
    const command = '/usr/share/fydeos_shell/license-utils.sh id';
    let licenseId = '';
    try {
      const result = await this.executeCommand_(command);
      licenseId = result.trim();
    } catch (err) {
      console.error(err);
      return '';
    }
    return licenseId;
  }

  popupRenew() {
   const url = `${CASHIER_SERVER_URL}/web/checkoutCounter.html?licenseId=${this.licenseId_}&hl=${this.language_}&board=${this.boardName_}`;
    window.open(url);
  }

  popupUpgrade() {
    const url = `${CASHIER_SERVER_URL}/web/upgradeCheckoutCounter.html?licenseId=${this.licenseId_}&hl=${this.language_}&board=${this.boardName_}`;
    window.open(url);
  }

}
