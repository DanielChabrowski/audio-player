find_package(Qt6 COMPONENTS Core Multimedia CONFIG REQUIRED)

set(SOURCES
    include/MediaPlayer.hpp
    qtbackend/MediaPlayerQtBackend.cpp
    qtbackend/MediaPlayerQtBackend.hpp
)

add_library(media ${SOURCES})
add_library(player::media ALIAS media)

target_link_libraries(
    media
    PRIVATE Qt6::Core Qt6::Multimedia
    PUBLIC player::core
)

target_include_directories(media PUBLIC include/)
