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

install(
    FILES share/foobar.desktop
    DESTINATION /usr/share/applications
)
