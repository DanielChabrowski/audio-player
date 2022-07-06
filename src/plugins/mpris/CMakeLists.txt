find_package(Qt6 REQUIRED COMPONENTS DBus)

include(FetchContent)

fetchcontent_declare(
    mpris-spec
    GIT_REPOSITORY https://gitlab.freedesktop.org/mpris/mpris-spec
    GIT_TAG 51e5848f9f763864568db233bffe98e3cb72bf13
)
fetchcontent_makeavailable(mpris-spec)

set(MPRIS_SPEC_DIR ${mpris-spec_SOURCE_DIR})

set(MPRIS_SPEC_FILES
    ${MPRIS_SPEC_DIR}/spec/org.mpris.MediaPlayer2.xml
    ${MPRIS_SPEC_DIR}/spec/org.mpris.MediaPlayer2.Player.xml
    #${MPRIS_SPEC_DIR}/spec/org.mpris.MediaPlayer2.Playlists.xml
    #${MPRIS_SPEC_DIR}/spec/org.mpris.MediaPlayer2.TrackList.xml
)

execute_process(
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/dbus-annotate-qt.py ${MPRIS_SPEC_FILES}
)

add_library(mpris-plugin STATIC MprisPlugin.hpp MprisPlugin.cpp)
add_library(plugins::mpris ALIAS mpris-plugin)

foreach(filename IN ITEMS ${MPRIS_SPEC_FILES})
    qt_add_dbus_adaptor(MPRIS_ADAPTOR ${filename} MprisPlugin.hpp)
    target_sources(mpris-plugin PRIVATE ${MPRIS_ADAPTOR})
endforeach()

target_link_libraries(mpris-plugin PUBLIC Qt6::DBus)
target_include_directories(mpris-plugin PUBLIC ${CMAKE_CURRENT_LIST_DIR})
