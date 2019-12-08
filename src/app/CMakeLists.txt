find_package(Qt5 COMPONENTS Widgets Multimedia CONFIG REQUIRED)

set(SOURCES
    main.cpp

    MainWindow.cpp
    MainWindow.hpp
    MainWindow.ui

    PlaylistWidget.cpp
    PlaylistWidget.hpp

    PlaylistHeader.cpp
    PlaylistHeader.hpp

    PlaylistModel.cpp
    PlaylistModel.hpp

    resources/Resources.qrc
)

add_executable(foobar ${SOURCES})

target_link_libraries(foobar
  PRIVATE
    Qt5::Widgets
    Qt5::Multimedia
    player::core
)
