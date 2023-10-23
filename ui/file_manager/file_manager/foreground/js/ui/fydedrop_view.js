// Copyright 2020 FydeOS Authors. All rights reserved.
// Author: yudong

import {loadTimeData} from 'chrome://resources/ash/common/load_time_data.m.js';

export class FydeDropView {
  constructor(fydeDropView) {
    /**
     * @private {!HTMLElement}
     */
    this.fydeDropView_ = fydeDropView;

    // Download permission is denied by default in a webview
    this.fydeDropView_.addEventListener('permissionrequest', function(e) {
      if (e.permission === 'download') {
        e.request.allow();
      }
    });
  }

  show() {
    const webview = this.fydeDropView_.getElementsByTagName('webview')[0];
    if (!webview.src) {
      webview.src = loadTimeData.getString('FYDE_DROP_URL');
    }
    this.fydeDropView_.hidden = false;
  }

  hide() {
    this.fydeDropView_.hidden = true;
  }
}
