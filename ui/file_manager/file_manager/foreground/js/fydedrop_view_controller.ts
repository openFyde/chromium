// Copyright 2020 FydeOS Authors. All rights reserved.
// Author: yudong

import {FydeDropView} from './ui/fydedrop_view.js';
import {FileManagerUI} from './ui/file_manager_ui.js';
import {DirectoryModel} from './directory_model.js';

export class FydeDropViewController {
  private fydeDropView_: FydeDropView;
  private directoryModel_: DirectoryModel;
  private ui_: FileManagerUI;
  /**
   * @param {!FydeDropView} fydeDropView FydeDrop view.
   * @param {!DirectoryModel} directoryModel Directory model.
   */
  constructor(ui: FileManagerUI, fydeDropView: FydeDropView, directoryModel: DirectoryModel) {
    console.log('FydeDropViewController constructor');
    this.ui_ = ui;
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

  hideActionButtonsAndTable_() {
    console.log('hideActionButtons_');
    this.ui_.sortButton.setAttribute('hidden', '');
    this.ui_.toggleViewButton.setAttribute('hidden', '');
    this.ui_.listContainer.table.setAttribute('hidden', '');
  }

  showActionButtonsAndTable_() {
    console.log('showActionButtons_');
    this.ui_.sortButton.removeAttribute('hidden');
    this.ui_.toggleViewButton.removeAttribute('hidden');
    this.ui_.listContainer.table.removeAttribute('hidden');
  }

  onFydeDropStarted_() {
    console.log('onFydeDropStarted_');
    this.hideActionButtonsAndTable_();
    this.fydeDropView_.show();
  }

  onFydeDropStopped_() {
    console.log('onFydeDropStopped_');
    this.fydeDropView_.hide();
    this.showActionButtonsAndTable_();
  }
}
