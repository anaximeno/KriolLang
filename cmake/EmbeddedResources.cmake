set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR})

set(KRIOL_WASI_TARGET "wasm32-wasi" CACHE STRING "Target triple used for Kriol WASI output")
set(KRIOL_WASI_SYSROOT "/usr" CACHE PATH "WASI sysroot used for Kriol WASI output")

set(RUNTIME_NATIVE_GC_BC     ${GENERATED_DIR}/kriol_runtime_native_gc.bc)
set(RUNTIME_NATIVE_GC_HEADER ${GENERATED_DIR}/kriol_runtime_native_gc.bc.h)
set(GC_NATIVE_HEADER         ${GENERATED_DIR}/libgc_native.h)
set(KRIOL_EMBEDDED_RESOURCE_HEADERS
    ${RUNTIME_NATIVE_GC_HEADER}
    ${GC_NATIVE_HEADER}
)

if(KRIOL_ENABLE_WASM)
    include(ExternalProject)

    if(KRIOL_WASI_ENABLE_GC)
        file(READ "${BDWGC_DIR}/include/private/gcconfig.h" KRIOL_BDWGC_GCCONFIG)
        string(FIND "${KRIOL_BDWGC_GCCONFIG}" "__wasi__" KRIOL_BDWGC_HAS_WASI_CONFIG)
        if(KRIOL_BDWGC_HAS_WASI_CONFIG EQUAL -1)
            message(FATAL_ERROR
                "KRIOL_WASI_ENABLE_GC=ON requires a Boehm GC checkout with wasm32-wasi support. "
                "The current stable Boehm GC checkout does not support wasm32-wasi; use the default "
                "WASI no-GC runtime or switch Boehm GC to a WASI-capable release/branch."
            )
        endif()

        set(RUNTIME_WASM32_WASI_BC     ${GENERATED_DIR}/kriol_runtime_wasm32_wasi_gc.bc)
        set(RUNTIME_WASM32_WASI_HEADER ${GENERATED_DIR}/kriol_runtime_wasm32_wasi_gc.bc.h)
        set(GC_WASM32_WASI_HEADER      ${GENERATED_DIR}/libgc_wasm32_wasi.h)
    else()
        set(RUNTIME_WASM32_WASI_BC     ${GENERATED_DIR}/kriol_runtime_wasm32_wasi_nogc.bc)
        set(RUNTIME_WASM32_WASI_HEADER ${GENERATED_DIR}/kriol_runtime_wasm32_wasi_nogc.bc.h)
    endif()

    set(WASI_CRT1_COMMAND_HEADER      ${GENERATED_DIR}/wasi_crt1_command.o.h)
    set(WASI_LIBC_HEADER              ${GENERATED_DIR}/wasi_libc.a.h)
    set(WASI_LIBM_HEADER              ${GENERATED_DIR}/wasi_libm.a.h)
    set(WASI_BUILTINS_HEADER          ${GENERATED_DIR}/wasi_builtins.a.h)
    list(APPEND KRIOL_EMBEDDED_RESOURCE_HEADERS
        ${RUNTIME_WASM32_WASI_HEADER}
        ${WASI_CRT1_COMMAND_HEADER}
        ${WASI_LIBC_HEADER}
        ${WASI_LIBM_HEADER}
        ${WASI_BUILTINS_HEADER}
    )

    if(KRIOL_WASI_ENABLE_GC)
        list(APPEND KRIOL_EMBEDDED_RESOURCE_HEADERS
            ${GC_WASM32_WASI_HEADER}
        )
    endif()
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
    set(WASI_LIB_DIR ${KRIOL_WASI_SYSROOT}/lib/wasm32-wasi)
    set(WASI_CRT1_COMMAND ${WASI_LIB_DIR}/crt1-command.o)
    set(WASI_LIBC ${WASI_LIB_DIR}/libc.a)
    set(WASI_LIBM ${WASI_LIB_DIR}/libm.a)

    execute_process(
        COMMAND
            ${CLANG_PROGRAM}
            --target=${KRIOL_WASI_TARGET}
            --sysroot=${KRIOL_WASI_SYSROOT}
            --print-libgcc-file-name
        OUTPUT_VARIABLE WASI_BUILTINS
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE WASI_BUILTINS_RESULT
    )

    if(NOT WASI_BUILTINS_RESULT EQUAL 0 OR NOT EXISTS "${WASI_BUILTINS}")
        message(FATAL_ERROR "Could not locate WASI compiler-rt builtins with ${CLANG_PROGRAM}")
    endif()

    foreach(_KRIOL_WASI_INPUT IN ITEMS
        "${WASI_CRT1_COMMAND}"
        "${WASI_LIBC}"
        "${WASI_LIBM}"
    )
        if(NOT EXISTS "${_KRIOL_WASI_INPUT}")
            message(FATAL_ERROR "Required WASI input not found: ${_KRIOL_WASI_INPUT}")
        endif()
    endforeach()

    add_custom_command(
        OUTPUT ${RUNTIME_WASM32_WASI_BC}

        COMMAND
            ${CLANG_PROGRAM}
            --target=${KRIOL_WASI_TARGET}
            --sysroot=${KRIOL_WASI_SYSROOT}
            -emit-llvm
            -O3
            -c
            ${CMAKE_SOURCE_DIR}/runtime/kriol_runtime.c
            -o
            ${RUNTIME_WASM32_WASI_BC}
            $<$<BOOL:${KRIOL_WASI_ENABLE_GC}>:-I${BDWGC_DIR}/include>
            -DKRIOL_RUNTIME_NO_GC=$<NOT:$<BOOL:${KRIOL_WASI_ENABLE_GC}>>

        DEPENDS
            ${CMAKE_SOURCE_DIR}/runtime/kriol_runtime.c
    )

    kriol_embed_file(${RUNTIME_WASM32_WASI_BC} ${RUNTIME_WASM32_WASI_HEADER})

    add_custom_command(
        OUTPUT ${WASI_CRT1_COMMAND_HEADER}
        COMMAND ${CMAKE_COMMAND} -E copy ${WASI_CRT1_COMMAND} ${GENERATED_DIR}/wasi_crt1_command.o
        COMMAND ${XXD_PROGRAM} -i wasi_crt1_command.o > ${WASI_CRT1_COMMAND_HEADER}
        WORKING_DIRECTORY ${GENERATED_DIR}
        DEPENDS ${WASI_CRT1_COMMAND}
    )

    add_custom_command(
        OUTPUT ${WASI_LIBC_HEADER}
        COMMAND ${CMAKE_COMMAND} -E copy ${WASI_LIBC} ${GENERATED_DIR}/wasi_libc.a
        COMMAND ${XXD_PROGRAM} -i wasi_libc.a > ${WASI_LIBC_HEADER}
        WORKING_DIRECTORY ${GENERATED_DIR}
        DEPENDS ${WASI_LIBC}
    )

    add_custom_command(
        OUTPUT ${WASI_LIBM_HEADER}
        COMMAND ${CMAKE_COMMAND} -E copy ${WASI_LIBM} ${GENERATED_DIR}/wasi_libm.a
        COMMAND ${XXD_PROGRAM} -i wasi_libm.a > ${WASI_LIBM_HEADER}
        WORKING_DIRECTORY ${GENERATED_DIR}
        DEPENDS ${WASI_LIBM}
    )

    add_custom_command(
        OUTPUT ${WASI_BUILTINS_HEADER}
        COMMAND ${CMAKE_COMMAND} -E copy ${WASI_BUILTINS} ${GENERATED_DIR}/wasi_builtins.a
        COMMAND ${XXD_PROGRAM} -i wasi_builtins.a > ${WASI_BUILTINS_HEADER}
        WORKING_DIRECTORY ${GENERATED_DIR}
        DEPENDS ${WASI_BUILTINS}
    )

    if(KRIOL_WASI_ENABLE_GC)
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
                -DBUILD_SHARED_LIBS=OFF
                -Denable_threads=OFF
                -Denable_docs=OFF
                -Dbuild_cord=OFF
                -Denable_cplusplus=OFF
                -Denable_gcj_support=OFF
                -Denable_java_finalization=OFF
                -Dbuild_tests=OFF
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
endif()

add_custom_target(
    kriol_embedded_resources
    DEPENDS
        ${KRIOL_EMBEDDED_RESOURCE_HEADERS}
)
