set(SOURCES
    ConfigurationKeys.hpp
    Playlist.cpp
    Playlist.hpp
    IPlaylistIO.hpp
    FilesystemPlaylistIO.cpp
    FilesystemPlaylistIO.hpp
    PlaylistManager.cpp
    PlaylistManager.hpp
    FileUtilities.cpp
    FileUtilities.hpp
)

add_library(core ${SOURCES})
add_library(player::core ALIAS core)

find_package(Qt6 COMPONENTS Core CONFIG REQUIRED)
target_link_libraries(core PUBLIC Qt6::Core player::metadata)

target_include_directories(core PUBLIC ${CMAKE_CURRENT_LIST_DIR})

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
