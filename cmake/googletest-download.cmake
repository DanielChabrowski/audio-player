cmake_minimum_required(VERSION 3.9)

project(googletest-download NONE)

include(ExternalProject)
externalproject_add(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG main
    SOURCE_DIR @GT_SOURCE_DIR@
    BINARY_DIR @GT_BUILD_DIR@
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    TEST_COMMAND ""
)
