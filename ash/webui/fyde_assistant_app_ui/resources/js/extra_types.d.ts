// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

interface LaunchParams {
  readonly files: readonly FileSystemHandle[];
}

interface LaunchQueue {
  setConsumer(consumer: (params: LaunchParams) => void): void;
}

type LaunchConsumer = (params: LaunchParams) => void;

interface LaunchQueue {
  setConsumer(consumer: LaunchConsumer): void;
}

interface Window {
  readonly launchQueue: LaunchQueue;
}

