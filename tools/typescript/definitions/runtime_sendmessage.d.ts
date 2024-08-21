
declare namespace chrome {
  export namespace runtime {
    export function sendMessage(
      extensionId: string,
      message: any,
      options?: object,
      callback?: (response: any) => void,
    ): void;
  }
}
