set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/arc)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Handle resources for Qt targets
set(CMAKE_AUTORCC ON)

option(FORCE_COLORED_OUTPUT "Force compiler diagnostic colors" OFF)
if(FORCE_COLORED_OUTPUT)
    add_compile_options($<$<CXX_COMPILER_ID:GNU>:-fdiagnostics-color=always>)
    add_compile_options($<$<CXX_COMPILER_ID:Clang>:-fcolor-diagnostics>)
endif()

include(cmake/sanitizers.cmake)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    add_compile_options(/permissive-)
else()
    add_compile_options(
        -Werror
        -Wall
        -Wextra
        -Wpedantic
        -Wredundant-decls
        -Wnon-virtual-dtor
        -Wnull-dereference
        -Wzero-as-null-pointer-constant
        $<$<CXX_COMPILER_ID:GNU>:-Wsuggest-override>
        $<$<CXX_COMPILER_ID:GNU>:-Wduplicated-branches>
        $<$<CXX_COMPILER_ID:GNU>:-Wlogical-op>
        $<$<CXX_COMPILER_ID:Clang>:-Wno-gnu-zero-variadic-macro-arguments>
    )

    # WA for a GCC bug
    # no way? to check for Multi-Config build type
    if(
        CMAKE_CXX_COMPILER_ID STREQUAL "GNU"
        AND (NOT CMAKE_BUILD_TYPE OR NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    )
        if(CMAKE_CXX_COMPILER_VERSION MATCHES "^12\.[012].*$")
            add_compile_options(-Wno-maybe-uninitialized)
        endif()
    endif()

    include(CheckIPOSupported)
    check_ipo_supported(RESULT IPO_SUPPORTED LANGUAGES CXX)

    include(CheckLinkerFlag)
    check_linker_flag(CXX "-fuse-ld=lld" LLD_SUPPORTED)
    if(
        LLD_SUPPORTED
        AND IPO_SUPPORTED
        AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang"
    )
        # lld doesn't work when used with gcc and lto enabled
        # https://github.com/llvm/llvm-project/issues/41791
        add_link_options("-fuse-ld=lld")
        message(STATUS "Using lld linker")
    else()
        check_linker_flag(CXX "-fuse-ld=gold" GOLD_SUPPORTED)
        if(GOLD_SUPPORTED)
            add_link_options("-fuse-ld=gold")
            message(STATUS "Using gold linker")
        endif()
    endif()
endif()
