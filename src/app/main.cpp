#include "ApplicationStyle.hpp"
#include "AudioMetaDataProvider.hpp"
#include "AudioMetaDataProviderCache.hpp"
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
    app.setStyle(new ApplicationStyle);
    app.setWindowIcon(QIcon{ ":/icons/icon.svg" });

#ifdef NDEBUG
    constexpr auto applicationName = "foobar";
#else
    constexpr auto applicationName = "foobar-debug";
#endif

    QSettings appSettings{ applicationName, applicationName };
    qInfo() << "Config file:" << appSettings.fileName();

    const auto configLocation =
        QStandardPaths::standardLocations(QStandardPaths::StandardLocation::ConfigLocation).at(0);
    qInfo() << "Config directory:" << configLocation;

    const auto cacheFile = QString{ "%1/%2/%3" }.arg(configLocation, applicationName, "cache.sqlite");

    AudioMetaDataProvider metaDataProvider;
    AudioMetaDataProviderCache metadataProviderCache(cacheFile, &metaDataProvider);
    FilesystemPlaylistIO playlistIO{ metadataProviderCache };

    const auto playlistsDirectory = QString{ "%1/%2/%3" }.arg(configLocation, applicationName, "playlists");
    qInfo() << "Playlists directory:" << playlistsDirectory;

    PlaylistManager playlistManager{ playlistIO, playlistsDirectory };

    MainWindow window{ appSettings, playlistManager };
    window.show();

    return app.exec();
}
