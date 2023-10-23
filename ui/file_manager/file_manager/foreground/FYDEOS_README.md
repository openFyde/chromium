# File Manager

## [Feature] Nutstore sign-up hint

> Maintainer: yudong@fydeos.io

Here are all the files that are modified or added to pop up the Nutstore sign-up hint when file manager is launched.

```
modified: chrome/browser/chromeos/file_manager/file_manager_string_util.cc
new file: fydeos/resources/ui/file_manager/file_manager/foreground/css/file_manager_patch.css
new file: fydeos/resources/ui/file_manager/file_manager/foreground/images/files/ui/2x/nutstore-steps.png
new file: fydeos/resources/ui/file_manager/file_manager/foreground/images/files/ui/nutstore-steps.png
new file: fydeos/resources/ui/file_manager/file_manager/foreground/js/ui/nutstore_hint_dialog.js
new file: fydeos/resources/ui/file_manager/file_manager/README.md
modified: ui/file_manager/file_manager/foreground/js/main_scripts.js
modified: ui/file_manager/file_manager/foreground/js/ui/file_manager_ui.js
modified: ui/file_manager/file_manager/main.html
modified: ui/file_manager/file_manager/manifest.json
```

The component mimics an existed one named `cloud-import-details`. Their DOM structures and styles are very similar (**main.html** and **file_manager_patch.css**).

The constructor function `NutstoreHintDialog` and some other events and behaviors are defined in **nutstore_hint_dialog.js**, and used in **file_manager_ui.js**.

Every new js file must be added to **main_scripts.js** to let the build knows something has been added, otherwise you will get `NutstoreHintDialog is not defined` error.

The **storage** permission is added in **manifest.json** in order to use `chrome.storage.local` api for saving a flag to display the dialog only once. And the **management** permission is added for launching the store app.

New items are added in i18n:

  - `NUTSTORE_HINT_BANNER_TITLE`
  - `NUTSTORE_HINT_BANNER_DESCRIPTION`
  - `NUTSTORE_HINT_CONFIRM_BUTTON_TEXT`
  - `NUTSTORE_HINT_STEPS_SIGUNUP`
  - `NUTSTORE_HINT_STEPS_ADD_APPLICATION`
  - `NUTSTORE_HINT_STEPS_CREATE_PASSWORD`
  - `NUTSTORE_HINT_STEPS_OPEN_NUTSTORE`
  - `NUTSTORE_HINT_STEPS_CONNECT_ACCOUNT`

## [Feature] FydeDrop in file manager

> Maintainer: yudong@fydeos.io

Files added or modified:

```
modified: chrome/browser/chromeos/file_manager/file_manager_string_util.cc
modified: fydeos/resources/ui/file_manager/file_manager/foreground/css/file_manager_patch.css
new file: fydeos/resources/ui/file_manager/file_manager/foreground/images/volumes/2x/fydedrop.png
new file: fydeos/resources/ui/file_manager/file_manager/foreground/images/volumes/2x/fydedrop_active.png
new file: fydeos/resources/ui/file_manager/file_manager/foreground/images/volumes/fydedrop.png
new file: fydeos/resources/ui/file_manager/file_manager/foreground/images/volumes/fydedrop_active.png
new file: fydeos/resources/ui/file_manager/file_manager/foreground/js/file_manager_insert_fydedrop.js
new file: fydeos/resources/ui/file_manager/file_manager/foreground/js/fydedrop_view_controller.js
new file: fydeos/resources/ui/file_manager/file_manager/foreground/js/navigation_list_model.js
new file: fydeos/resources/ui/file_manager/file_manager/foreground/js/ui/directory_tree.js
new file: fydeos/resources/ui/file_manager/file_manager/foreground/js/ui/fydedrop_view.js
modified: fydeos/resources/ui/fydeos_ui_resources.grdp
modified: fydeos/resources/ui_chromeos_strings_zh-CN.xtb
modified: ui/file_manager/base/js/volume_manager_types.js
modified: ui/file_manager/file_manager/background/js/volume_manager_impl.js
modified: ui/file_manager/file_manager/common/js/util.js
modified: ui/file_manager/file_manager/foreground/js/directory_model.js
modified: ui/file_manager/file_manager/foreground/js/file_manager.js
modified: ui/file_manager/file_manager/foreground/js/main_scripts.js
modified: ui/file_manager/file_manager/foreground/js/navigation_list_model.js
modified: ui/file_manager/file_manager/foreground/js/ui/directory_tree.js
modified: ui/file_manager/file_manager/foreground/js/ui/file_manager_ui.js
modified: ui/file_manager/file_manager/main.html
```

Memo:

  - To change the breadcrumb label, check `ui/file_manager/file_manager/foreground/js/toolbar_controller.js`.
