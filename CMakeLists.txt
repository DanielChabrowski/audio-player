cmake_minimum_required(VERSION 3.7)

project(foobar CXX)

include(cmake/build.cmake)
include(cmake/sanitizers.cmake)

find_package(Qt5Widgets CONFIG REQUIRED)
find_package(Qt5Multimedia CONFIG REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(Taglib taglib REQUIRED IMPORTED_TARGET)

set(SOURCES
    src/main.cpp

    src/MainWindow.cpp
    src/MainWindow.hpp
    src/MainWindow.ui

    src/PlaylistWidget.cpp
    src/PlaylistWidget.hpp

    src/PlaylistModel.cpp
    src/PlaylistModel.hpp

    src/PlaylistHeader.cpp
    src/PlaylistHeader.hpp

    src/PlaylistLoader.cpp
    src/PlaylistLoader.hpp

    src/IAudioMetaDataProvider.hpp
    src/AudioMetaDataProvider.cpp
    src/AudioMetaDataProvider.hpp

    src/AudioPropertyReader.hpp

    src/TaglibAudioPropertyReader.cpp
    src/TaglibAudioPropertyReader.hpp

    src/Song.hpp
    src/Playlist.hpp

    resources/Resources.qrc
)

add_executable(foobar ${SOURCES})

target_link_libraries(foobar
  PRIVATE
    Qt5::Widgets
    Qt5::Multimedia
    PkgConfig::Taglib
)
