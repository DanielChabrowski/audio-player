cmake_minimum_required(VERSION 3.7)

project(foobar CXX)

include(cmake/build.cmake)
include(cmake/sanitizers.cmake)

find_package(Qt5Widgets CONFIG REQUIRED)
find_package(Qt5Multimedia CONFIG REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(Taglib taglib REQUIRED IMPORTED_TARGET)

set(SOURCES
    MainWindow.cpp
    MainWindow.hpp
    MainWindow.ui

    PlaylistWidget.cpp
    PlaylistWidget.hpp

    PlaylistModel.cpp
    PlaylistModel.hpp

    PlaylistHeader.cpp
    PlaylistHeader.hpp

    Song.hpp
    Playlist.hpp

    Resources.qrc
)

add_executable(foobar main.cpp MainWindow.ui ${SOURCES})
target_link_libraries(foobar Qt5::Widgets Qt5::Multimedia PkgConfig::Taglib)
