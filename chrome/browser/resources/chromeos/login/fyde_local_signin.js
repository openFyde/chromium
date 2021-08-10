// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

(function() {

Polymer({
  is: 'fyde-local-signin-element',

  behaviors: [ LoginScreenBehavior ],

  ready() {
    this.initializeLoginScreen('FydeLocalSigninScreen', {
      resetAllowed: true,
    });
  },

  onBackClicked() {
    this.userActed('cancel');
  },

});

})();
