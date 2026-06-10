export interface KriolModule {
  HEAPU8: Uint8Array;
  HEAPU32: Uint32Array;
  ccall(
    ident: string,
    returnType: string | null,
    argTypes: string[],
    args: unknown[]
  ): unknown;
  UTF8ToString(ptr: number): string;
  _malloc(size: number): number;
  _free(ptr: number): void;
  _kriol_free(ptr: number): void;
}

export interface KriolModuleOptions {
  locateFile?: (path: string, prefix: string) => string;
}

export default function createKriolModule(
  options?: KriolModuleOptions
): Promise<KriolModule>;
