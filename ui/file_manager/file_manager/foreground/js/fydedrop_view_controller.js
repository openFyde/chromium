// Copyright 2020 FydeOS Authors. All rights reserved.
// Author: yudong

import {FydeDropView} from './ui/fydedrop_view.js';
import {DirectoryModel} from './directory_model.js';

export class FydeDropViewController {
  /**
   * @param {!FydeDropView} fydeDropView FydeDrop view.
   * @param {!DirectoryModel} directoryModel Directory model.
   */
  constructor(fydeDropView, directoryModel) {
    console.log('FydeDropViewController constructor');
    /**
     * @private {!FydeDropView}
     */
    this.fydeDropView_ = fydeDropView;

    /**
     * @private {!DirectoryModel}
     */
    this.directoryModel_ = directoryModel;

    this.directoryModel_.addEventListener(
        'fydedrop-started', this.onFydeDropStarted_.bind(this));
    this.directoryModel_.addEventListener(
        'fydedrop-stopped', this.onFydeDropStopped_.bind(this));
  }

  hideActionButtons_() {
    console.log('hideActionButtons_');
    fileManager.ui.sortButton.setAttribute('hidden', '');
    fileManager.ui.toggleViewButton.setAttribute('hidden', '');
  }

  showActionButtons_() {
    console.log('showActionButtons_');
    fileManager.ui.sortButton.removeAttribute('hidden');
    fileManager.ui.toggleViewButton.removeAttribute('hidden');
  }

  onFydeDropStarted_() {
    console.log('onFydeDropStarted_');
    this.hideActionButtons_();
    this.fydeDropView_.show();
  }

  onFydeDropStopped_() {
    console.log('onFydeDropStopped_');
    this.fydeDropView_.hide();
    this.showActionButtons_();
  }
}
