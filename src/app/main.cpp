#include "ApplicationStyle.hpp"
#include "AudioMetaDataProvider.hpp"
#include "FilesystemPlaylistIO.hpp"
#include "MainWindow.hpp"
#include "PlaylistManager.hpp"

#include <QApplication>
#include <QDebug>
#include <QSettings>
#include <QStandardPaths>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setStyle(new ApplicationStyle);

    constexpr auto applicationName = "foobar";
    QSettings appSettings{ applicationName, applicationName };
    qInfo() << "Configuration file:" << appSettings.fileName();

    AudioMetaDataProvider metaDataProvider;
    FilesystemPlaylistIO playlistIO{ metaDataProvider };

    const auto configLocation =
        QStandardPaths::standardLocations(QStandardPaths::StandardLocation::ConfigLocation).at(0);
    const auto playlistsDirectory =
        QString{ "%1/%2/%3" }.arg(configLocation).arg(applicationName).arg("playlists");
    qInfo() << "Playlists directory:" << playlistsDirectory;

    PlaylistManager playlistManager{ playlistIO, playlistsDirectory };

    MainWindow window{ appSettings, playlistManager };
    window.show();

    return app.exec();
}
