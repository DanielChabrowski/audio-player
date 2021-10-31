find_package(Qt6 COMPONENTS Core Multimedia CONFIG REQUIRED)

set(SOURCES MediaPlayer.hpp MediaPlayerQtBackend.cpp MediaPlayerQtBackend.hpp)

add_library(media ${SOURCES})
add_library(player::media ALIAS media)

target_link_libraries(media PRIVATE Qt6::Core Qt6::Multimedia)

target_include_directories(media PUBLIC ${CMAKE_CURRENT_LIST_DIR})
