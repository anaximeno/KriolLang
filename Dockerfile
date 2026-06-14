ARG UBUNTU_TAG=resolute
FROM ubuntu:${UBUNTU_TAG}

ARG LLVM_VERSION=20
ARG CMAKE_BUILD_TYPE=Debug

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        ca-certificates \
        git \
        make \
        cmake \
        flex \
        bison \
        xxd \
        file \
        zlib1g-dev \
        libzstd-dev \
        pkg-config \
        clang-${LLVM_VERSION} \
        llvm-${LLVM_VERSION} \
        llvm-${LLVM_VERSION}-dev \
        llvm-${LLVM_VERSION}-tools \
        lld-${LLVM_VERSION} \
        liblld-${LLVM_VERSION} \
        liblld-${LLVM_VERSION}-dev \
        libclang-rt-${LLVM_VERSION}-dev-wasm32 \
        wasi-libc \
    && ln -sf /usr/bin/clang-${LLVM_VERSION} /usr/local/bin/clang \
    && ln -sf /usr/bin/clang++-${LLVM_VERSION} /usr/local/bin/clang++ \
    && ln -sf /usr/bin/llvm-config-${LLVM_VERSION} /usr/local/bin/llvm-config \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src/kriol

COPY . .

RUN test -f include/external/bdwgc/CMakeLists.txt \
    || (echo "Missing include/external/bdwgc. Run 'git submodule update --init include/external/bdwgc' before docker build." >&2; exit 1)

RUN test -f /usr/lib/llvm-${LLVM_VERSION}/include/lld/Common/Driver.h \
    && test -f /usr/lib/llvm-${LLVM_VERSION}/lib/liblldCommon.a \
    && test -f /usr/lib/llvm-${LLVM_VERSION}/lib/liblldWasm.a \
    && test -f /usr/lib/wasm32-wasi/crt1-command.o \
    && test -f /usr/lib/wasm32-wasi/libc.a \
    && test -f /usr/lib/wasm32-wasi/libm.a \
    && test -f "$(clang-${LLVM_VERSION} --target=wasm32-wasi --sysroot=/usr --print-libgcc-file-name)"

RUN cmake -S . -B build/resolute \
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} \
        -DKRIOL_ENABLE_WASM=ON \
        -DKRIOL_USE_EMBEDDED_LLD=ON \
        -DCMAKE_C_COMPILER=clang-${LLVM_VERSION} \
        -DCMAKE_CXX_COMPILER=clang++-${LLVM_VERSION} \
        -DLLVM_DIR=/usr/lib/llvm-${LLVM_VERSION}/lib/cmake/llvm \
    && cmake --build build/resolute --target check

RUN build/resolute/kriol examples/hello_world.kriol \
        --target wasm32-wasi \
        -o /tmp/kriol_hello.wasm \
    && file /tmp/kriol_hello.wasm \
    && llvm-nm-${LLVM_VERSION} --defined-only /tmp/kriol_hello.wasm \
        | grep -E '(^|[[:space:]])_start($|[[:space:]])'

CMD ["build/resolute/kriol", "--help"]
