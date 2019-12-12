#include "ApplicationStyle.hpp"
#include "AudioMetaDataProvider.hpp"
#include "FilesystemPlaylistIO.hpp"
#include "MainWindow.hpp"
#include "PlaylistManager.hpp"

#include <QApplication>
#include <QSettings>
#include <QStandardPaths>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setStyle(new ApplicationStyle);

    QSettings appSettings{ "OpenSource", "Foobar3000" };

    AudioMetaDataProvider metaDataProvider{};
    FilesystemPlaylistIO playlistIO{ metaDataProvider };

    const auto playlistDirPath =
        QStandardPaths::standardLocations(QStandardPaths::StandardLocation::ConfigLocation).at(0) + "/playlists";
    PlaylistManager playlistManager{ playlistIO, playlistDirPath };

    MainWindow window{ appSettings, playlistManager };
    window.show();

    return app.exec();
}
