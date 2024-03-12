
import {ChromeEvent} from './chrome_event.js';

declare global {
  export namespace chrome {
    export namespace shellClient {

      export const ON_PROCESS: number;
      export const ON_CLOSED: number; 
      export const ON_ERROR: number;
      export const ON_NONE: number;

      export interface ExecState {
        code: number;
        result: string;
      }

      export function execSync(cmd: string, callback: (state: ExecState) => void): void;
      export function execAsync(cmd: string, callback: (state: ExecState) => void): void;
      export function getTaskOutput(key: number, lines: number, callback: (state: ExecState) => void): void;
      export function forceCloseTask(key: number, callback: (state: ExecState) => void): void;

      export interface ShellCommandNotifyingEvent extends ChromeEvent<(
        key: number,
        state: number,
        message: string) => void> { }

      export const onShellCommandNotifying: ShellCommandNotifyingEvent;
    }
  }
}
