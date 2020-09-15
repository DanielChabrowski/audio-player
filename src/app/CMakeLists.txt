find_package(Qt5 COMPONENTS Widgets Multimedia CONFIG REQUIRED)

set(SOURCES
    main.cpp

    ApplicationStyle.cpp
    ApplicationStyle.hpp

    MainWindow.cpp
    MainWindow.hpp

    PlaylistWidget.cpp
    PlaylistWidget.hpp

    PlaylistHeader.cpp
    PlaylistHeader.hpp

    PlaylistModel.cpp
    PlaylistModel.hpp

    EscapableLineEdit.cpp
    EscapableLineEdit.hpp

    MultilineTabBar.cpp
    MultilineTabBar.hpp

    resources/Resources.qrc
)

add_executable(foobar ${SOURCES})

target_link_libraries(foobar
  PRIVATE
    Qt5::Widgets
    Qt5::Multimedia
    player::core
)

install(
    TARGETS foobar
    RUNTIME DESTINATION bin
)

install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/icon.svg
    DESTINATION /usr/share/icons/foobar
)

install(
    FILES ${CMAKE_SOURCE_DIR}/share/foobar.desktop
    DESTINATION /usr/share/applications
)

add_custom_target(
    run
    COMMAND foobar
    USES_TERMINAL
)
