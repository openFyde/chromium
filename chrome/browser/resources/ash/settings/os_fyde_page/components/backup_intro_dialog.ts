import 'chrome://resources/cr_elements/cr_dialog/cr_dialog.js';
import 'chrome://resources/cr_components/localized_link/localized_link.js';
import {I18nMixin} from 'chrome://resources/cr_elements/i18n_mixin.js';
import {PolymerElement, mixinBehaviors} from 'chrome://resources/polymer/v3_0/polymer/polymer_bundled.min.js';
import {CrDialogElement} from '//resources/cr_elements/cr_dialog/cr_dialog.js';

import {getTemplate} from './backup_intro_dialog.html.js';

interface BackupIntroDialog {
  $: {
    dialog: CrDialogElement,
  };
}

const BackupIntroDialogElementBase = I18nMixin(PolymerElement);

class BackupIntroDialog extends BackupIntroDialogElementBase {
  static get is() {
    return 'backup-intro-dialog';
  }

  static get template() {
    return getTemplate();
  }

  static get properties() {
    return {};
  }

  override ready() {
    super.ready();
    console.log('backup intro dialog ready');
  }

  override connectedCallback() {
    super.connectedCallback();
    this.$.dialog.showModal();
  }

  onDialogCancel_() {
    console.log('cancel');
    this.$.dialog.cancel();
  }

  onDialogConfirm_() {
    console.log('confirm');
    this.$.dialog.close();
  }
}

customElements.define(
    BackupIntroDialog.is, BackupIntroDialog);
