cmake_minimum_required(VERSION 3.7)

project(foobar CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/arc)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)
# Handle resources for Qt targets
set(CMAKE_AUTORCC ON)

find_package(Qt5Widgets CONFIG REQUIRED)
find_package(Qt5Multimedia CONFIG REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(Taglib taglib REQUIRED IMPORTED_TARGET)

add_compile_options(
    -Werror
    -Wextra
    -Wpedantic
)

set(SOURCES
    MainWindow.cpp
    MainWindow.hpp
    MainWindow.ui

    PlaylistWidget.cpp
    PlaylistWidget.hpp

    PlaylistModel.cpp
    PlaylistModel.hpp

    Song.hpp
    Playlist.hpp

    Resources.qrc
)

add_executable(foobar main.cpp MainWindow.ui ${SOURCES})
target_link_libraries(foobar Qt5::Widgets Qt5::Multimedia PkgConfig::Taglib)
