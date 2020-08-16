cmake_minimum_required(VERSION 3.7)

project(foobar CXX)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

option(BUILD_TESTING "Build tests" ON)
if(BUILD_TESTING)
    include(googletest)
    enable_testing()
endif()

find_package(Qt5 COMPONENTS Core CONFIG REQUIRED)

include(build)

add_subdirectory(src)

set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Daniel Chabrowski")
set(CPACK_DEBIAN_PACKAGE_DEPENDS
    "
    libqt5widgets5,
    libqt5multimedia5,
    libtag1v5-vanilla,
    gstreamer1.0-plugins-base,
    gstreamer1.0-plugins-good,
    gstreamer1.0-plugins-ugly
    "
)

include(CPack)
