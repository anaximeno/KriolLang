import { pathToFileURL } from 'node:url';

const modulePath = process.argv[2];
if (!modulePath) {
  console.error('usage: node tests/browser/compile-smoke.mjs <path-to-kriol.js>');
  process.exit(1);
}

const moduleUrl = pathToFileURL(modulePath);
const { default: createKriolModule } = await import(moduleUrl.href);
const module = await createKriolModule({
  locateFile(path) {
    return new URL(path, moduleUrl).pathname;
  },
});

const source = `fn inisiu() {
    mostran("Kuale, Mundu!");
}
`;

const sourceBytes = new TextEncoder().encode(source);
const sourcePtr = module._malloc(sourceBytes.length + 1);
const wasmOutPtr = module._malloc(4);
const wasmLenPtr = module._malloc(4);
const errorPtr = module._malloc(4);

module.HEAPU8.set(sourceBytes, sourcePtr);
module.HEAPU8[sourcePtr + sourceBytes.length] = 0;
module.HEAPU32[wasmOutPtr >> 2] = 0;
module.HEAPU32[wasmLenPtr >> 2] = 0;
module.HEAPU32[errorPtr >> 2] = 0;

try {
  const code = module.ccall(
    'kriol_compile_source_to_wasm',
    'number',
    ['number', 'number', 'number', 'number', 'number'],
    [sourcePtr, sourceBytes.length, wasmOutPtr, wasmLenPtr, errorPtr],
  );

  const errorValue = module.HEAPU32[errorPtr >> 2];
  if (code !== 0) {
    const message = errorValue ? module.UTF8ToString(errorValue) : 'unknown error';
    throw new Error(message);
  }

  const wasmPtr = module.HEAPU32[wasmOutPtr >> 2];
  const wasmLen = module.HEAPU32[wasmLenPtr >> 2];
  const wasm = module.HEAPU8.slice(wasmPtr, wasmPtr + wasmLen);
  module._kriol_free(wasmPtr);

  if (
    wasm.length < 4 ||
    wasm[0] !== 0x00 ||
    wasm[1] !== 0x61 ||
    wasm[2] !== 0x73 ||
    wasm[3] !== 0x6d
  ) {
    throw new Error('compiler did not return a WebAssembly module');
  }
} finally {
  const errorValue = module.HEAPU32[errorPtr >> 2];
  if (errorValue)
    module._kriol_free(errorValue);
  module._free(sourcePtr);
  module._free(wasmOutPtr);
  module._free(wasmLenPtr);
  module._free(errorPtr);
}
