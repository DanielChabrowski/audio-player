cmake_minimum_required(VERSION 3.7)

project(foobar CXX)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

option(BUILD_TESTING "Build tests" ON)

find_package(Qt5 COMPONENTS Widgets Multimedia CONFIG REQUIRED)
find_package(Taglib REQUIRED)

if(BUILD_TESTING)
    include(googletest)
endif()

include(build)

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
    Taglib::Taglib
)

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
