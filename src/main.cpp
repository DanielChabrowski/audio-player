#include "AudioMetaDataProvider.hpp"
#include "MainWindow.hpp"
#include "PlaylistLoader.hpp"
#include "PlaylistManager.hpp"

#include <QApplication>
#include <QSettings>
#include <QStandardPaths>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QSettings appSettings{ "OpenSource", "Foobar3000" };

    AudioMetaDataProvider metaDataProvider{};
    PlaylistLoader playlistLoader{ metaDataProvider };

    const auto playlistDirPath =
    QStandardPaths::standardLocations(QStandardPaths::StandardLocation::ConfigLocation).at(0) +
    "/playlists";
    PlaylistManager playlistManager{ playlistLoader, playlistDirPath };

    MainWindow window{ appSettings, playlistManager };
    window.show();

    return app.exec();
}
