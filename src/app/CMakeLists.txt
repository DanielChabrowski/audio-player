find_package(Qt5 COMPONENTS Widgets Multimedia CONFIG REQUIRED)

set(SOURCES
    main.cpp

    ApplicationStyle.cpp
    ApplicationStyle.hpp

    MainWindow.cpp
    MainWindow.hpp
    MainWindow.ui

    PlaylistWidget.cpp
    PlaylistWidget.hpp

    PlaylistHeader.cpp
    PlaylistHeader.hpp

    PlaylistModel.cpp
    PlaylistModel.hpp

    EscapableLineEdit.cpp
    EscapableLineEdit.hpp

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
    FILES share/foobar.desktop
    DESTINATION /usr/share/applications
)

add_custom_target(
    run
    COMMAND foobar
    USES_TERMINAL
)
