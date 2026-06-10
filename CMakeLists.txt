cmake_minimum_required(VERSION 3.20)

# -----------------------------------------------------------------------------
# Compiler selection
# -----------------------------------------------------------------------------

find_program(_CLANG NAMES clang-20 clang-19 clang)
find_program(_CLANGPP NAMES clang++-20 clang++-19 clang++)

if(_CLANG)
    set(CMAKE_C_COMPILER "${_CLANG}")
endif()

if(_CLANGPP)
    set(CMAKE_CXX_COMPILER "${_CLANGPP}")
endif()

project(kriol LANGUAGES C CXX)

# -----------------------------------------------------------------------------
# Build configuration
# -----------------------------------------------------------------------------

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

option(KRIOL_STATIC "Statically link the kriol binary in Release builds" OFF)
option(KRIOL_ENABLE_WASM "Build the kriol compiler with wasm32-wasi output support" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------------------------------------------------------------
# Tools
# -----------------------------------------------------------------------------

find_program(CLANG_PROGRAM NAMES clang-20 clang-19 clang REQUIRED)
find_program(XXD_PROGRAM xxd REQUIRED)
find_program(VALGRIND valgrind)

# -----------------------------------------------------------------------------
# LLVM
# -----------------------------------------------------------------------------

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")

set(KRIOL_LLVM_COMPONENTS
    core
    native
    linker
    bitreader
)

if(KRIOL_ENABLE_WASM)
    list(APPEND KRIOL_LLVM_COMPONENTS
        WebAssemblyCodeGen
        WebAssemblyDesc
        WebAssemblyInfo
        WebAssemblyUtils
    )
endif()

llvm_map_components_to_libnames(
    LLVM_LIBS
    ${KRIOL_LLVM_COMPONENTS}
)

separate_arguments(
    LLVM_DEFINITIONS_LIST
    UNIX_COMMAND
    "${LLVM_DEFINITIONS}"
)

list(FILTER LLVM_DEFINITIONS_LIST EXCLUDE REGEX "^-fno-exceptions$")
list(FILTER LLVM_DEFINITIONS_LIST EXCLUDE REGEX "^-std=")

# -----------------------------------------------------------------------------
# Boehm GC
# -----------------------------------------------------------------------------

include(cmake/BoehmGC.cmake)

# -----------------------------------------------------------------------------
# Bison / Flex
# -----------------------------------------------------------------------------

find_package(BISON)
find_package(FLEX)

if(BISON_FOUND AND FLEX_FOUND)

    BISON_TARGET(
        KriolParser
        ${CMAKE_SOURCE_DIR}/rules/parser.y
        ${CMAKE_BINARY_DIR}/parser.cc
        DEFINES_FILE ${CMAKE_BINARY_DIR}/parser.hh
        COMPILE_FLAGS "-dt"
    )

    FLEX_TARGET(
        KriolScanner
        ${CMAKE_SOURCE_DIR}/rules/scanner.l
        ${CMAKE_BINARY_DIR}/scanner.cc
    )

    ADD_FLEX_BISON_DEPENDENCY(KriolScanner KriolParser)

    set(PARSER_SRC ${BISON_KriolParser_OUTPUTS})
    set(SCANNER_SRC ${FLEX_KriolScanner_OUTPUTS})

else()

    message(STATUS "Using pre-generated parser/scanner")

    set(PARSER_SRC  ${CMAKE_SOURCE_DIR}/parser.cc)
    set(SCANNER_SRC ${CMAKE_SOURCE_DIR}/scanner.cc)

endif()

# -----------------------------------------------------------------------------
# Runtime
# -----------------------------------------------------------------------------

add_library(
    kriol_runtime
    OBJECT
    runtime/kriol_runtime.c
)

target_include_directories(
    kriol_runtime
    BEFORE PRIVATE
    ${BDWGC_DIR}/include
)

# -----------------------------------------------------------------------------
# Sources
# -----------------------------------------------------------------------------

set(KRIOL_CORE_SOURCES
    main.cc
    src/kriol/sema.cc
    ${PARSER_SRC}
    ${SCANNER_SRC}
)

set(KRIOL_LLVM_SOURCES
    src/kriol/cli.cc
    src/kriol/codegen.cc
)

# -----------------------------------------------------------------------------
# Embedded resources
# -----------------------------------------------------------------------------

include(cmake/EmbeddedResources.cmake)

# -----------------------------------------------------------------------------
# Executable
# -----------------------------------------------------------------------------

add_executable(
    kriol
    ${KRIOL_CORE_SOURCES}
    ${KRIOL_LLVM_SOURCES}
)

set_source_files_properties(
    ${KRIOL_LLVM_SOURCES}
    PROPERTIES COMPILE_OPTIONS "${LLVM_DEFINITIONS_LIST}"
)

add_dependencies(kriol kriol_embedded_resources)

target_include_directories(
    kriol
    BEFORE PRIVATE
    ${BDWGC_DIR}/include
)

target_include_directories(
    kriol
    PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${LLVM_INCLUDE_DIRS}
)

target_compile_options(
    kriol
    PRIVATE
    $<$<CONFIG:Debug>:-g -Wall>
    $<$<CONFIG:Release>:-O2 -DNDEBUG>
)

target_compile_definitions(
    kriol
    PRIVATE
    KRIOL_ENABLE_WASM=$<BOOL:${KRIOL_ENABLE_WASM}>
    KRIOL_WASI_TARGET="${KRIOL_WASI_TARGET}"
    KRIOL_WASI_SYSROOT="${KRIOL_WASI_SYSROOT}"
)

# -----------------------------------------------------------------------------
# Linking
# -----------------------------------------------------------------------------

if(KRIOL_STATIC AND CMAKE_BUILD_TYPE STREQUAL "Release")
    list(FILTER LLVM_LIBS EXCLUDE REGEX "z3")
    target_link_options(
        kriol
        PRIVATE
        -static-libgcc
        -static-libstdc++
    )
endif()

target_link_libraries(
    kriol
    PRIVATE
    ${LLVM_LIBS}
    gc
    m
)

# -----------------------------------------------------------------------------
# Installation
# -----------------------------------------------------------------------------

install(TARGETS kriol DESTINATION bin)

install(
    FILES $<TARGET_OBJECTS:kriol_runtime>
    DESTINATION bin/runtime
    RENAME kriol_runtime.o
)

# -----------------------------------------------------------------------------
# Utility targets
# -----------------------------------------------------------------------------

if(VALGRIND)

    add_custom_target(
        valgrind
        COMMAND ${VALGRIND}
                --leak-check=full
                --show-leak-kinds=all
                --error-exitcode=1
                $<TARGET_FILE:kriol>
                "$ENV{KRIOL_FILE}"
        DEPENDS kriol
    )

endif()

add_custom_target(
    check
    COMMAND
        ${CMAKE_SOURCE_DIR}/run_tests.sh
        $<TARGET_FILE:kriol>
        ${CMAKE_SOURCE_DIR}
    DEPENDS kriol
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
