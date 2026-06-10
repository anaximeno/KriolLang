# 🦈 KriolLang

**KriolLang** or **Kriol** is a programming language based on the **Cape Verdean Creole**. It was made to be easy for anyone who understands Cape Verdean Creole while keeping the same versatility as some other languages, such as C, Go, and Rust.

```kriol
fn ola(textu nomi, bool naKriolu) {
    si naKriolu {
        mostran(f"Olá {nomi}, ami nta programa na Kriolu!");
    } sinon {
        mostran(f"Hello {nomi}, I program in Kriol!");
    }
}

fn inisiu() {
    ola("Visitanti", sin);
    ola("Guest", nau);
}
```

### Description

The syntax of this programming language was initially based on C, Go, and Rust, but other programming languages such as Liquid played a good role in the initial design.

For more concrete specifications about the language, you are recommended to read the [KriolLang Language Specification](./docs/kriol-lang-spec.md).

The file extension of the KriolLang programming language is `.kr` or `.kriol` and the name of the lang's compiler is **kriol**.

# Install

The easiest way to test it currently is using a linux-based OS (if you're on Windows I would recommend trying it inside WSL), and as a dependency at least make sure to have a cc linker available like `gcc` or `clang` to ensure proper linkage of the produced object file `.o` into the binary code required to run in the system.

You can install it using the following command:

```sh
curl -fsSL https://raw.githubusercontent.com/kriol-lang/kriol/refs/heads/main/install.sh | sh -s -- --pre
```

After that add the following to  your shell env:

```sh
export PATH="$HOME/.kriol:$PATH"
```

Then close and reopen the shell and try `kriol --version`.


## Usage

The first thing to do after compiling the compiler is to execute the help command:

```sh
./kriol --help
```

After executing that command you may see the usage section and some of its options.

An example of how it could be used is to compile one of the example files in the examples folder:

```sh
./kriol examples/fibonacci-recursive.kriol
```

This compiles `fibonacci-recursive.kriol` to a native binary named `a.out` by default. Use `-o` to specify a different output name:

```sh
./kriol examples/fibonacci-recursive.kriol -o fibonacci-recursive
```

Then execute the compiled binary with:

```sh
./fibonacci-recursive
```

To inspect the generated LLVM IR instead of producing a binary:

```sh
./kriol examples/example03.kriol --emit-ir
```

To compile a Kriol program to a WASI WebAssembly module:

```sh
./kriol examples/hello-world.kriol --target wasm32-wasi -o hello.wasm
```

The generated module is a WASI command module. For a browser playground, run it
through a WASI preview1 JavaScript shim and capture stdout to display
`mostra`/`mostran` output.

Formatted strings use the embedded Boehm GC runtime, including for WASI output.

## Build

If you want to build the project, currently it only works mostly on Linux based operating systems. It depends mainly on **Clang/LLVM 20** and **LLD 20** to compile the source code and link user programs, and **bison** and **flex** to compile the language rules.

If you are on a Debian (or Ubuntu) based Linux operating system you can install the dependencies using the following command:

```sh
apt install make flex bison clang-20 llvm-20 llvm-20-dev llvm-20-tools lld-20 libgc-dev zlib1g-dev libzstd-dev xxd cmake
```

WASI output also needs the WASI libc and compiler runtime packages:

```sh
apt install wasi-libc libclang-rt-20-dev-wasm32
```

WASM support is enabled by default when building `kriol`. If you want a
native-only compiler with fewer build dependencies, configure with:

```sh
cmake -B build -DKRIOL_ENABLE_WASM=OFF
```

After installing the dependencies, compile the compiler using:

```sh
cmake -B build && cmake --build build
```

And then run with:

```sh
./build/kriol --help
```

For a release build use:

```sh
cmake -B build/release -DCMAKE_BUILD_TYPE=Release -DKRIOL_STATIC=ON && cmake --build build/release
```

And run with:

```sh
./build/release/kriol --help
```

## Tests

You can run tests with the following command:

```sh
cmake -B build && cmake --build build --target check
```

<!--
## Visuals

TODO...

## Contributing

TODO...

-->

## License

All the code and specifications of the KriolLang programming language are currently under the [MIT License](./LICENSE).
