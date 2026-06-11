set(BDWGC_DIR "${CMAKE_SOURCE_DIR}/include/external/bdwgc")

if(NOT EXISTS "${BDWGC_DIR}/CMakeLists.txt")
    message(FATAL_ERROR
        "Missing Boehm GC submodule.\n"
        "Run:\n"
        "git submodule update --init include/external/bdwgc"
    )
endif()

if(KRIOL_STATIC AND CMAKE_BUILD_TYPE STREQUAL "Release")
    set(_GC_SHARED OFF)
else()
    set(_GC_SHARED ON)
endif()

if(DEFINED BUILD_SHARED_LIBS)
    set(_KRIOL_OLD_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})
    set(_KRIOL_HAD_BUILD_SHARED_LIBS ON)
else()
    set(_KRIOL_HAD_BUILD_SHARED_LIBS OFF)
endif()

set(BUILD_SHARED_LIBS ${_GC_SHARED} CACHE BOOL "" FORCE)

set(enable_docs              OFF CACHE BOOL "" FORCE)
set(enable_gcj_support       OFF CACHE BOOL "" FORCE)
set(enable_java_finalization OFF CACHE BOOL "" FORCE)
set(enable_threads           ON  CACHE BOOL "" FORCE)

add_subdirectory(
    ${BDWGC_DIR}
    _bdwgc
    EXCLUDE_FROM_ALL
)

if(_KRIOL_HAD_BUILD_SHARED_LIBS)
    set(BUILD_SHARED_LIBS ${_KRIOL_OLD_BUILD_SHARED_LIBS} CACHE BOOL "" FORCE)
else()
    unset(BUILD_SHARED_LIBS CACHE)
endif()

message(STATUS "Boehm GC shared=${_GC_SHARED}")
