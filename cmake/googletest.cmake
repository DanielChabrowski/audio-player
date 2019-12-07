set(GT_DIR ${CMAKE_BINARY_DIR}/googletest-download)
set(GT_SOURCE_DIR ${CMAKE_BINARY_DIR}/googletest-src)
set(GT_BUILD_DIR ${CMAKE_BINARY_DIR}/googletest-build)

configure_file(
    cmake/googletest-download.cmake
    ${GT_DIR}/CMakeLists.txt
    @ONLY
)

execute_process(
    COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
    WORKING_DIRECTORY ${GT_DIR}
    OUTPUT_QUIET
)

execute_process(
    COMMAND ${CMAKE_COMMAND} --build .
    WORKING_DIRECTORY ${GT_DIR}
    OUTPUT_QUIET
)

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

add_subdirectory(
    ${CMAKE_CURRENT_BINARY_DIR}/googletest-src
    ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
    EXCLUDE_FROM_ALL
)

add_library(GTest::GTest ALIAS gtest)
add_library(GTest::Main ALIAS gtest_main)
