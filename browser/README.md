# Kriol Browser Compiler

Experimental notes for the browser-facing Kriol compiler module:

- `kriol.js`
- `kriol.wasm`
- `kriol.d.ts`

The module exposes a small C ABI through Emscripten. The intended playground
flow is to call `kriol_compile_source_to_wasm`, copy the returned wasm bytes out
of the module heap, then release returned pointers with `kriol_free`.

This path is intentionally not part of the release workflow yet. The playground
currently targets the backend compile service first, while this module remains a
future client-side option.
