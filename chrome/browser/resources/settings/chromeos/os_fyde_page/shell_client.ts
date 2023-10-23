// fydeos shellclient

type ShellClientAsyncCallback = {
  successCB?: (message: string) => void,
  faildCB?: (err: Error) => void,
  intervalCB?: (message: string) => void,
  timeout?: number,
  interval?: number,
  outputLines?: number,
}

export class ShellClient {
  private element_: HTMLElement;

  constructor(element: HTMLElement) {
    this.element_ = element;
  }

  async ExecForResult(cmd: string) {
    return new Promise((resolve, reject) => {
      chrome.shellClient.execSync(cmd, (ret) => {
        // console.log('shellClient cmd', cmd);
        // console.log('shellClient result', ret);
        const { code, result } = ret;
        if (code !== 0) {
          reject(result);
        } else {
          resolve(result);
        }
      });
    })
  }

  ExecForLongTask(cmd: string, {
    successCB, faildCB, intervalCB,
    timeout = 600, interval = 1, outputLines = 1,
  }: ShellClientAsyncCallback) {
    console.log(`command '${cmd}' will be executed`);
    chrome.shellClient.execAsync(cmd, (result) => {
      if (result.code === -1) {
        if (faildCB) {
          faildCB(new Error(result.result));
        }
        return;
      }
      let taskFinish = false;
      const checkRunningState = (key: number, state: number, message: string) => {
        if (key !== result.code) return;
        if (taskFinish) {
          chrome.shellClient.onShellCommandNotifying.removeListener(checkRunningState);
          return;
        }
        switch (state) {
          case chrome.shellClient.ON_ERROR:
            chrome.shellClient.getTaskOutput(key, 1, (obj) => {
              if (faildCB) {
                faildCB(new Error(obj ? obj.result : message));
              }
            });
            taskFinish = true;
            break;
          case chrome.shellClient.ON_CLOSED:
            if (successCB) {
              successCB(message);
            }
            taskFinish = true;
            break;
          case chrome.shellClient.ON_PROCESS:
            // ignore on_process here
            break;
          default:
            console.log('onShellCommandNotifying, unexpected status, state: ', state);
            console.log('onShellCommandNotifying, unexpected status, message: ', message);
            if (faildCB) {
              faildCB(new Error(message || 'Task may be terminated by others'));
            }
            taskFinish = true;
        }
      };
      chrome.shellClient.onShellCommandNotifying.addListener(checkRunningState);

      let counter = Math.round(timeout / interval);
      function checkOutput() {
        if (taskFinish) return;
        counter -= 1;
        let end = false;
        if (counter >= 0) {
          chrome.shellClient.getTaskOutput(result.code, outputLines, (obj) => {
            switch (obj.code) {
              case chrome.shellClient.ON_PROCESS:
                if (intervalCB) {
                  intervalCB(obj.result);
                }
                break;
              case chrome.shellClient.ON_CLOSED:
              case chrome.shellClient.ON_ERROR:
              case chrome.shellClient.ON_NONE:
                // ignore onclose, onerror here
                break;
              default:
                console.log('getTaskOutput, unexpected', obj);
                if (faildCB && !taskFinish) {
                  faildCB(new Error('Task may be terminated by others'));
                }
                end = true;
                break;
            }
            if (end) {
              return;
            }
            setTimeout(checkOutput, interval * 1000);
          });
        } else {
          chrome.shellClient.forceCloseTask(result.code, () => {
            if (faildCB) {
              faildCB(new Error('The mission is force closed for timeout!'));
            }
          });
        }
      }
      checkOutput();
    });
  }

  async SafeExecForResult(cmd: string) {
    let result: string | unknown;
    try {
      result = await this.ExecForResult(cmd);
    } catch (err) {
      console.error(`shellClient execForResult cmd [${cmd}] error`, err);
    }
    if (result && typeof result === 'string') {
      result = result.trim();
      return result as string;
    }
    return '';
  }

  async IsFileExist(filename: string) {
    let exist = false;
    try {
      await this.ExecForResult(`ls ${filename}`)
      exist = true;
    } catch (err) {
      exist = false;
    }
    return exist;
  }

  async Sleep(second: number) {
    return new Promise<void>(resolve => {
      setTimeout(() => {
        resolve();
      }, second * 1000);
    });
  }

  dispatchEvent(type: string, custom: object | undefined) {
    if (!this.element_) return;
    this.element_.dispatchEvent(new CustomEvent(type, custom));
  }
}
