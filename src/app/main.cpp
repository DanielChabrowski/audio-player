#include "ApplicationStyle.hpp"
#include "AudioMetaDataProvider.hpp"
#include "FilesystemPlaylistIO.hpp"
#include "MainWindow.hpp"
#include "MetaDataCache.hpp"
#include "PlaylistManager.hpp"

#include <QApplication>
#include <QDebug>
#include <QDir>
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
        QStandardPaths::writableLocation(QStandardPaths::StandardLocation::ConfigLocation);

    if(configLocation.isEmpty())
    {
        qCritical() << "Could not find directory suitable for writing configuration";
        return 1;
    }

    qInfo() << "Config directory:" << configLocation;

    QDir configDir{ configLocation };
    configDir.mkdir(applicationName);

    const auto cacheFile = QString{ "%1/%2/%3" }.arg(configLocation, applicationName, "cache.db");

    MetaDataCache metaDataCache{ cacheFile };
    AudioMetaDataProvider metaDataProvider;
    FilesystemPlaylistIO playlistIO{ metaDataCache, metaDataProvider };

    const auto playlistsDirectory = QString{ "%1/%2/%3" }.arg(configLocation, applicationName, "playlists");
    qInfo() << "Playlists directory:" << playlistsDirectory;

    PlaylistManager playlistManager{ playlistIO, playlistsDirectory };

    MainWindow window{ appSettings, playlistManager };
    window.show();

    return app.exec();
}
