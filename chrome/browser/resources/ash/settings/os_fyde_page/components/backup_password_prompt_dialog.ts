import '//resources/ash/common/cr_elements/cr_button/cr_button.js';
import '//resources/ash/common/cr_elements/cr_dialog/cr_dialog.js';
import '//resources/ash/common/cr_elements/cr_input/cr_input.js';
import '//resources/ash/common/cr_elements/cr_shared_style.css.js';
import '../../settings_shared.css.js';

import {PolymerElement, mixinBehaviors} from 'chrome://resources/polymer/v3_0/polymer/polymer_bundled.min.js';
import {CrDialogElement} from '//resources/ash/common/cr_elements/cr_dialog/cr_dialog.js';
import {CrInputElement} from '//resources/ash/common/cr_elements/cr_input/cr_input.js';

import {getTemplate} from './backup_password_prompt_dialog.html.js';

interface BackupPasswordPromptDialog {
  $: {
    dialog: CrDialogElement,
  };
}

const BackupPasswordPromptDialogElementBase = mixinBehaviors(
    [], PolymerElement);

class BackupPasswordPromptDialog extends BackupPasswordPromptDialogElementBase {
  static get is() {
    return 'backup-password-prompt-dialog';
  }

  static get template() {
    return getTemplate();
  }

  static get properties() {
    return {
      inputValue_: {
        type: String,
        value: '',
        observer: 'onInputValueChange_',
      },

      passwordInvalid_: {
        type: Boolean,
        value: false,
      },

      quickUnlockPrivate: {type: Object, value: chrome.quickUnlockPrivate},

      waitingForPasswordCheck_: {
        type: Boolean,
        value: false,
      },
    };
  }

  private inputValue_: string;
  private passwordInvalid_: boolean;
  quickUnlockPrivate: typeof chrome.quickUnlockPrivate;
  private waitingForPasswordCheck_: boolean;

  get passwordInput(): CrInputElement {
    return this.shadowRoot!.querySelector('cr-input')!;
  }

  override connectedCallback() {
    super.connectedCallback();

    this.$.dialog.showModal();
    // This needs to occur at the next paint otherwise the password input will
    // not receive focus.
    window.setTimeout(() => {
      this.passwordInput.focus();
    }, 1);
  }

  onCancelTap_() {
    if (this.$.dialog.open) {
      this.$.dialog.cancel();
    }
    console.log('cancel');
  }

  submitPassword_() {
    this.waitingForPasswordCheck_ = true;

    const password = this.passwordInput.value;
    // The user might have started entering a password and then deleted it all.
    // Do not submit/show an error in this case.
    if (!password) {
      this.passwordInvalid_ = false;
      this.waitingForPasswordCheck_ = false;
      return;
    }

    this.quickUnlockPrivate.getAuthToken(password, () => {
      this.waitingForPasswordCheck_ = false;
      if (chrome.runtime.lastError) {
        this.passwordInvalid_ = true;
        // Select the whole password if user entered an incorrect password.
        this.passwordInput.select();
        return;
      }

      this.dispatchEvent(new CustomEvent(
          'backup-password-obtained',
          {bubbles: true, composed: true, detail: password}));
      this.passwordInvalid_ = false;

      if (this.$.dialog.open) {
        this.$.dialog.close();
      }
    });
    console.log('confirm');
  }

  onInputValueChange_() {
    this.passwordInvalid_ = false;
  }

  isConfirmEnabled_() {
    return !this.waitingForPasswordCheck_ && !this.passwordInvalid_ &&
        this.inputValue_;
  }
}

customElements.define(
    BackupPasswordPromptDialog.is, BackupPasswordPromptDialog);
