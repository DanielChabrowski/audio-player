cmake_minimum_required(VERSION 3.7)

project(foobar CXX)

option(BUILD_TESTING "Build tests" ON)

find_package(Qt5 COMPONENTS Widgets Multimedia CONFIG REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(Taglib taglib REQUIRED IMPORTED_TARGET)

include(cmake/build.cmake)

if(BUILD_TESTING)
    include(cmake/googletest.cmake)
endif()

include(cmake/sanitizers.cmake)
include(cmake/warnings.cmake)

set(SOURCES
    src/main.cpp

    src/MainWindow.cpp
    src/MainWindow.hpp
    src/MainWindow.ui

    src/ConfigurationKeys.hpp

    src/Playlist.cpp
    src/Playlist.hpp

    src/PlaylistWidget.cpp
    src/PlaylistWidget.hpp

    src/PlaylistModel.cpp
    src/PlaylistModel.hpp

    src/PlaylistHeader.cpp
    src/PlaylistHeader.hpp

    src/PlaylistLoader.cpp
    src/PlaylistLoader.hpp

    src/PlaylistManager.cpp
    src/PlaylistManager.hpp

    src/AudioMetaData.hpp
    src/IAudioMetaDataProvider.hpp
    src/AudioMetaDataProvider.cpp
    src/AudioMetaDataProvider.hpp

    src/FileUtilities.cpp
    src/FileUtilities.hpp

    resources/Resources.qrc
)

add_executable(foobar ${SOURCES})

target_link_libraries(foobar
  PRIVATE
    Qt5::Widgets
    Qt5::Multimedia
    PkgConfig::Taglib
)

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
