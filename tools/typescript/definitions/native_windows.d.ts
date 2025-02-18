// Copyright 2021 Fyde Innovations. All rights reserved.

/** @fileoverview Definitions for chrome.nativeWindows API */

declare namespace chrome {
  export namespace nativeWindows {
    export enum WindowType {
      normal = 'normal',
      popup = 'popup',
      panel = 'panel',
      app = 'app',
      devtools = 'devtools',
    }

    export enum WindowState {
      normal = 'normal',
      minimized = 'minimized',
      maximized = 'maximized',
      fullscreen = 'fullscreen',
      docked = 'docked',
      locked_fullscreen = 'locked-fullscreen',
    }

    export interface Window {
      id?: number;
      focused: boolean;
      top?: number;
      left?: number;
      width?: number;
      height?: number;
      incognito: boolean;
      type?: WindowType;
      state?: WindowState;
      alwaysOnTop: boolean;
      sessionId?: string;
      appId?: string;
      launchId?: string
    }

    export function create(appId: string, callback?: (window?: Window) => void): void;

    export function getAll(callback: (windows: Window[]) => void): void;
  }
}
