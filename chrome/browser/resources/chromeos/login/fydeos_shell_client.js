// Encapsulate shellClient

class FydeOSShellClient {
  constructor() {
    console.log('FydeOSShellClient constructor');
    this.shellClient_ = chrome.shellClient;
  }

  runCommand(command) {
    return new Promise((resolve, reject) => {
      this.shellClient_.execSync(command, (response) => {
        const { result, code } = response;
        if (code !== 0) {
          reject(new Error(result));
        } else {
          resolve(result);
        }
      });
    });
  }

  async fileExists(file) {
    const ret = await this.runCommand(`ls ${file}`);
    return ret !== '';
  }

  async sleep(sec) {
    return new Promise(resolve => {
      setTimeout(() => {
        resolve();
      }, sec * 1000);
    });
  }

  async runCommandAsync(command, callback) {
    return new Promise((resolve, reject) => {
      this.shellClient_.execAsync(command, (response) => {
        const { code, result } = response;
        if (code === -1) {
          reject(new Error(result));
        } else {
          resolve(code);
        }
      });
    });
  }

  async getOutputOrResult(commandKey) {
    const lines = 10;
    return new Promise((resolve, reject) => {
      this.shellClient_.getTaskOutput(commandKey, lines, (response) => {
        const { code, result } = response;
        switch (code) {
          case this.shellClient_.ON_PROCESS:
            resolve({ result, closed: false });
          case this.shellClient_.ON_CLOSED:
            resolve({ code, closed: true });
            break;
          case this.shellClient_.ON_ERROR:
            reject(new Error(result));
            break;
          case this.shellClient_.ON_NONE:
            reject(new Error('The task has been terminated.'));
            break;
          default:
            reject(new Error('Task may be terminated by others'));
            break;
        }
      });
    });
  }

  async forceCloseTask(commandKey) {
    this.shellClient_.forceCloseTask(commandKey, (response) => {
      console.log('forceCloseTask', response);
    });
  }

  async getTaskState(key) {
    return new Promise((resolve, reject) => {
      this.shellClient_.getTaskState(key, (response) => {
        const { code, result } = response;
        if (code !== key) {
          reject(new Error('invalid response'));
        } else {
          try {
            const state = JSON.parse(result);
            resolve(state);
          } catch (e) {
            reject(e);
          }
        }
      });
    });
  }
}

export { FydeOSShellClient };
