# :stars: KriolLang

**KriolLang** or **Kriol** is a programming language based on the **Cape Verdian Creole**. It was made to be easy for anyone who understands Cape Verdean Creole while keeping the same versatility as some other languages, such as C, Go, and Rust.

### Project status

Under development.

### Description

The syntax of this programming language was initially based on C, Go, and Rust, but other programming languages such as Liquid played a good role in the initial design.

For more concrete specifications about the language, you are recommended to read the [KriolLang Language Specification](./docs/kriol-lang-spec.md).

The file extension of the KriolLang programming language is **.kl** (representing the word **Kriol** or **Kriolu** which is how Kriol or Creole is pronounced in Cape Verdian's Creole) and the compiler's name is **kriol**.

## Installation

It isn't installable on any OS yet, but it still can be tested using a Unix-based operating system. It depends on **clang++** or **g++** to compile the source code, and **bison** and **flex** to compile the language rules.

If you are on a Debian (or Ubuntu) based Linux operating system you can install the dependencies using the following command:

```bash
apt install make flex bison g++ clang llvm-19 llvm-19-dev
```

NOTE: now pre-generated parser and scanner codes are provided in the repository, so, **flex** and **bison** can be omitted from the installation above, but if you are planning to make changes to the parser or scanner rules, they will still be necessary.

After installing the dependencies, compile the compiler using:

```bash
make
```

For a release build use:

```bash
make release
```

The result of the compilation will be an executable named **kriol** which you can execute using:

```bash
./kriol
```

## Usage

The first thing to do after compiling the compiler is to execute the help command:

```bash
./kriol --help
```

After executing that command you may see the usage section and some of its options.

An example of how it could be used is to compile one of the example files in the examples folder:

```bash
./kriol examples/example03.kl
```

This compiles `example03.kl` to a native binary named `a.out` by default. Use `-o` to specify a different output name:

```bash
./kriol examples/example03.kl -o example03
```

Then execute the compiled binary with:

```bash
./example03
```

To inspect the generated LLVM IR instead of producing a binary:

```bash
./kriol examples/example03.kl --emit-ir
```
<!--
## Visuals

TODO...

## Contributing

TODO...

-->

## License

All the code and specifications of the KriolLang programming language are currently under the [MIT License](./LICENSE).
