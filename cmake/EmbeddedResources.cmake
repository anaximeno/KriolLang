set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR})

set(KRIOL_WASI_TARGET "wasm32-wasi" CACHE STRING "Target triple used for Kriol WASI output")
set(KRIOL_WASI_SYSROOT "/usr" CACHE PATH "WASI sysroot used for Kriol WASI output")

set(RUNTIME_NATIVE_GC_BC          ${GENERATED_DIR}/kriol_runtime_native_gc.bc)
set(RUNTIME_NATIVE_GC_HEADER        ${GENERATED_DIR}/kriol_runtime_native_gc.bc.h)
set(GC_NATIVE_HEADER      ${GENERATED_DIR}/libgc_native.h)
set(KRIOL_EMBEDDED_RESOURCE_HEADERS
    ${RUNTIME_NATIVE_GC_HEADER}
    ${GC_NATIVE_HEADER}
)

if(KRIOL_ENABLE_WASM)
    include(ExternalProject)

    set(RUNTIME_WASM32_WASI_GC_BC     ${GENERATED_DIR}/kriol_runtime_wasm32_wasi_gc.bc)
    set(RUNTIME_WASM32_WASI_GC_HEADER ${GENERATED_DIR}/kriol_runtime_wasm32_wasi_gc.bc.h)
    set(GC_WASM32_WASI_HEADER         ${GENERATED_DIR}/libgc_wasm32_wasi.h)
    list(APPEND KRIOL_EMBEDDED_RESOURCE_HEADERS
        ${RUNTIME_WASM32_WASI_GC_HEADER}
        ${GC_WASM32_WASI_HEADER}
    )
endif()

add_custom_command(
    OUTPUT ${RUNTIME_NATIVE_GC_BC}

    COMMAND
        ${CLANG_PROGRAM}
        -emit-llvm
        -O3
        -c
        ${CMAKE_SOURCE_DIR}/runtime/kriol_runtime.c
        -o
        ${RUNTIME_NATIVE_GC_BC}
        -I${BDWGC_DIR}/include

    DEPENDS
        ${CMAKE_SOURCE_DIR}/runtime/kriol_runtime.c
)

function(kriol_embed_file INPUT_FILE OUTPUT_HEADER)
    get_filename_component(INPUT_NAME ${INPUT_FILE} NAME)
    add_custom_command(
        OUTPUT ${OUTPUT_HEADER}

        COMMAND
            ${XXD_PROGRAM}
            -i
            ${INPUT_NAME}
            > ${OUTPUT_HEADER}

        WORKING_DIRECTORY ${GENERATED_DIR}

        DEPENDS
            ${INPUT_FILE}
    )
endfunction()

kriol_embed_file(${RUNTIME_NATIVE_GC_BC} ${RUNTIME_NATIVE_GC_HEADER})

add_custom_command(
    OUTPUT ${GC_NATIVE_HEADER}

    COMMAND
        ${CMAKE_COMMAND}
        -E copy
        $<TARGET_FILE:gc>
        ${GENERATED_DIR}/libgc_native.a

    COMMAND
        ${XXD_PROGRAM}
        -i
        libgc_native.a
        > ${GC_NATIVE_HEADER}

    WORKING_DIRECTORY ${GENERATED_DIR}

    DEPENDS
        gc
)

if(KRIOL_ENABLE_WASM)
    add_custom_command(
        OUTPUT ${RUNTIME_WASM32_WASI_GC_BC}

        COMMAND
            ${CLANG_PROGRAM}
            --target=${KRIOL_WASI_TARGET}
            --sysroot=${KRIOL_WASI_SYSROOT}
            -emit-llvm
            -O3
            -c
            ${CMAKE_SOURCE_DIR}/runtime/kriol_runtime.c
            -o
            ${RUNTIME_WASM32_WASI_GC_BC}
            -I${BDWGC_DIR}/include

        DEPENDS
            ${CMAKE_SOURCE_DIR}/runtime/kriol_runtime.c
    )

    kriol_embed_file(${RUNTIME_WASM32_WASI_GC_BC} ${RUNTIME_WASM32_WASI_GC_HEADER})

    set(WASI_GC_BUILD_DIR ${GENERATED_DIR}/_bdwgc_wasm32_wasi_cross)
    set(WASI_GC_LIB ${WASI_GC_BUILD_DIR}/libgc.a)

    ExternalProject_Add(
        kriol_bdwgc_wasm32_wasi
        SOURCE_DIR ${BDWGC_DIR}
        BINARY_DIR ${WASI_GC_BUILD_DIR}
        CMAKE_ARGS
            -DCMAKE_SYSTEM_NAME=WASI
            -DCMAKE_SYSTEM_PROCESSOR=wasm32
            -DCMAKE_C_COMPILER=${CLANG_PROGRAM}
            -DCMAKE_C_COMPILER_TARGET=${KRIOL_WASI_TARGET}
            -DCMAKE_SYSROOT=${KRIOL_WASI_SYSROOT}
            -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY
            -DGC_BUILD_SHARED_LIBS=OFF
            -Denable_threads=OFF
            -Denable_docs=OFF
            -Dbuild_cord=OFF
            -Denable_cplusplus=OFF
            -Denable_gcj_support=OFF
            -Denable_java_finalization=OFF
            -DBUILD_TESTING=OFF
        BUILD_BYPRODUCTS ${WASI_GC_LIB}
        INSTALL_COMMAND ""
    )

    add_custom_command(
        OUTPUT ${GC_WASM32_WASI_HEADER}

        COMMAND
            ${CMAKE_COMMAND}
            -E copy
            ${WASI_GC_LIB}
            ${GENERATED_DIR}/libgc_wasm32_wasi.a

        COMMAND
            ${XXD_PROGRAM}
            -i
            libgc_wasm32_wasi.a
            > ${GC_WASM32_WASI_HEADER}

        WORKING_DIRECTORY ${GENERATED_DIR}

        DEPENDS
            kriol_bdwgc_wasm32_wasi
    )
endif()

add_custom_target(
    kriol_embedded_resources
    DEPENDS
        ${KRIOL_EMBEDDED_RESOURCE_HEADERS}
)
