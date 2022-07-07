#include "ApplicationStyle.hpp"
#include "AudioMetaDataProvider.hpp"
#include "FilesystemPlaylistIO.hpp"
#include "LibraryManager.hpp"
#include "MainWindow.hpp"
#include "MediaPlayer.hpp"
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
    qInfo() << "Cache file:" << cacheFile;

    MetaDataCache metaDataCache{ cacheFile };
    AudioMetaDataProvider metaDataProvider;
    FilesystemPlaylistIO playlistIO{ metaDataCache, metaDataProvider };

    const auto playlistsDirectory = QString{ "%1/%2/%3" }.arg(configLocation, applicationName, "playlists");
    qInfo() << "Playlists directory:" << playlistsDirectory;

    PlaylistManager playlistManager{ playlistIO, playlistsDirectory };
    LibraryManager libraryManager{ metaDataCache };

    const auto mediaPlayer = MediaPlayer::create();
    if(not mediaPlayer)
    {
        qCritical() << "Could not create a media player backend";
        return 1;
    }

    MainWindow window{ appSettings, libraryManager, playlistManager, *mediaPlayer };
    window.show();

    return app.exec();
}
