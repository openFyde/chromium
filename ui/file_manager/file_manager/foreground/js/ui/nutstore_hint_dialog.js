// Copyright 2019 FydeOS Authors. All rights reserved.
// Author: yudong

const NUTSTORE_SIGNUP_URL = 'https://www.jianguoyun.com/d/signup';
const NUTSTORE_APPID = 'kpnkdhagknjilnbjjakakopieodoiifn';
const NUTSTORE_DISPLAY_ONCE_KEY = 'nutstoreDisplayOnce';

/**
 * @param {HTMLElement} parentNode Node to be parent for this dialog.
 * @constructor
 */
function NutstoreHintDialog(element) {
  if (!loadTimeData.getBoolean('FYDE_ACCOUNT_ENABLED')) return;

  const NUTSTORE_HINT_BANNER_TITLE = str('NUTSTORE_HINT_BANNER_TITLE');
  const NUTSTORE_HINT_BANNER_DESCRIPTION = str('NUTSTORE_HINT_BANNER_DESCRIPTION');
  const NUTSTORE_HINT_STEPS_SIGUNUP = str('NUTSTORE_HINT_STEPS_SIGUNUP');
  const NUTSTORE_HINT_STEPS_ADD_APPLICATION = str('NUTSTORE_HINT_STEPS_ADD_APPLICATION');
  const NUTSTORE_HINT_STEPS_CREATE_PASSWORD = str('NUTSTORE_HINT_STEPS_CREATE_PASSWORD');
  const NUTSTORE_HINT_STEPS_OPEN_NUTSTORE = str('NUTSTORE_HINT_STEPS_OPEN_NUTSTORE');
  const NUTSTORE_HINT_STEPS_CONNECT_ACCOUNT = str('NUTSTORE_HINT_STEPS_CONNECT_ACCOUNT');
  const NUTSTORE_HINT_CONFIRM_BUTTON_TEXT = str('NUTSTORE_HINT_CONFIRM_BUTTON_TEXT');

  const template = `<div>
    <div class="banner">
      <div class="title">${NUTSTORE_HINT_BANNER_TITLE}</div>
      <div class="description">${NUTSTORE_HINT_BANNER_DESCRIPTION}</div>
    </div>
    <div class="main">
      <div class="image"></div>
      <ol class="steps">
        <li><url class="clickable signup">${NUTSTORE_HINT_STEPS_SIGUNUP}</url></li>
        <li>${NUTSTORE_HINT_STEPS_ADD_APPLICATION}</li>
        <li>${NUTSTORE_HINT_STEPS_CREATE_PASSWORD}</li>
        <li><url class="clickable app-launch">${NUTSTORE_HINT_STEPS_OPEN_NUTSTORE}</url> ${NUTSTORE_HINT_STEPS_CONNECT_ACCOUNT}</li>
      </ol>
      <cr-button class="confirm" tabindex="0">
        <span>${NUTSTORE_HINT_CONFIRM_BUTTON_TEXT}</span>
      </cr-button>
    </div>
  </div>`

  // The dialog itself
  this.dialog_ =
    queryRequiredElement('#nutstore-hint-dialog', element);

  // Append html template to #nutstore-hint-dialog
  // Direct append to document.body will cause i18n error,
  // so #nutstore-hint-dialog node is required in main.html
  this.dialog_.innerHTML += template;

  // Hide the dialog
  this.cancelButton_ =
    queryRequiredElement('#nutstore-hint-dialog .confirm', element);
  this.cancelButton_.onclick = () => {
    this.dialog_.hidden = true;
  };

  // Visit nutstore signup page
  this.signupLink_ =
    queryRequiredElement('#nutstore-hint-dialog .signup', element);
  this.signupLink_.onclick = () => {
    window.open(NUTSTORE_SIGNUP_URL);
  };

  // Launch nutstore extension
  this.appLaunch_ =
    queryRequiredElement('#nutstore-hint-dialog .app-launch', element);
  this.appLaunch_.onclick = () => {
    chrome.management.getAll(apps => {
      const nutstoreApp = apps.find(item => item.id === NUTSTORE_APPID);
      if (nutstoreApp) {
        chrome.management.launchApp(NUTSTORE_APPID);
      } else {
        console.warn('Nutstore is not installed');
        window.open(`https://store.fydeos.com/?appid=${NUTSTORE_APPID}`);
      }
    });
  };

  // Display this dialog after 3 second after Files launched
  // and only once
  chrome.storage.local.get([NUTSTORE_DISPLAY_ONCE_KEY], (result) => {
    if (!result[NUTSTORE_DISPLAY_ONCE_KEY]) {
      let values = {};
      values[NUTSTORE_DISPLAY_ONCE_KEY] = 1;
      chrome.storage.local.set(values);

      setTimeout(() => {
        this.dialog_.hidden = false;
      }, 3000);
    }
  });
}
