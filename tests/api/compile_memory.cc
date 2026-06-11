#include "include/kriol/cli.hh"

#include <iostream>

int main() {
    kriol::cli::CompileOptions options;
    options.inputKind = kriol::cli::CompileInputKind::SourceText;
    options.input =
        "fn inisiu() {\n"
        "    mostran(\"Kuale, Mundu!\");\n"
        "}\n";
    options.sourceName = "<api-memory-test>";
    options.target = "wasm32-wasi";
    options.outputToMemory = true;

    kriol::cli::CompileResult result = kriol::cli::Compile(options);
    if (!result.diagnostics.empty()) {
        for (const auto& diagnostic : result.diagnostics)
            std::cerr << diagnostic << '\n';
        return 1;
    }

    if (!result.outputPath.empty()) {
        std::cerr << "expected in-memory compile to leave outputPath empty\n";
        return 1;
    }

    const auto& bytes = result.outputBytes;
    if (bytes.size() < 4 ||
        bytes[0] != 0x00 ||
        bytes[1] != 0x61 ||
        bytes[2] != 0x73 ||
        bytes[3] != 0x6d) {
        std::cerr << "expected in-memory compile to return a WebAssembly module\n";
        return 1;
    }

    return 0;
}
