#include "ApplicationStyle.hpp"
#include "AudioMetaDataProvider.hpp"
#include "FilesystemPlaylistIO.hpp"
#include "LibraryManager.hpp"
#include "MainWindow.hpp"
#include "MediaPlayer.hpp"
#include "MetaDataCache.hpp"
#include "PlaylistManager.hpp"

#ifdef PLUGIN_MPRIS_ENABLED
#include "MprisPlugin.hpp"
#endif

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QSettings>
#include <QStandardPaths>
#include <QStyleFactory>

// int main(int argc, char *argv[])
int main()
{
    // QApplication app(argc, argv);
    //  app.setStyle(QStyleFactory::create("kvantum"));
    // app.setStyle(new ApplicationStyle(QApplication::style()));
    // app.setWindowIcon(QIcon{ ":/icons/icon.svg" });

    // qDebug() << QStyleFactory::keys();

#ifdef NDEBUG
    constexpr auto applicationName = "foobar";
#else
    constexpr auto applicationName = "foobar-debug";
#endif

    QSettings appSettings{ applicationName, applicationName };
    qInfo() << "Config file:" << QDir::toNativeSeparators(appSettings.fileName());

    const auto configLocation =
        QStandardPaths::writableLocation(QStandardPaths::StandardLocation::ConfigLocation);

    if(configLocation.isEmpty())
    {
        qCritical() << "Could not find directory suitable for writing configuration";
        return 1;
    }

    qInfo() << "Config directory:" << QDir::toNativeSeparators(configLocation);

    QDir configDir{ configLocation };
    configDir.mkdir(applicationName);

    const auto cacheFile = QString{ "%1/%2/%3" }.arg(configLocation, applicationName, "cache.db");
    qInfo() << "Cache file:" << QDir::toNativeSeparators(cacheFile);

    MetaDataCache metaDataCache{ cacheFile };
    AudioMetaDataProvider metaDataProvider;
    FilesystemPlaylistIO playlistIO{ metaDataCache, metaDataProvider };

    const auto playlistsDirectory = QString{ "%1/%2/%3" }.arg(configLocation, applicationName, "playlists");
    qInfo() << "Playlists directory:" << QDir::toNativeSeparators(playlistsDirectory);

    PlaylistManager playlistManager{ playlistIO, playlistsDirectory };
    LibraryManager libraryManager{ metaDataCache };

    //     const auto mediaPlayer = MediaPlayer::create();
    //     if(not mediaPlayer)
    //     {
    //         qCritical() << "Could not create a media player backend";
    //         return 1;
    //     }

    // #ifdef PLUGIN_MPRIS_ENABLED
    //     plugins::MprisPlugin mprisPlugin(*mediaPlayer);
    // #endif

    //     MainWindow window{ appSettings, libraryManager, playlistManager, *mediaPlayer };
    //     window.show();

    // return app.exec();
    return 0;
}
