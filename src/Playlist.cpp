#include "Playlist.hpp"

#include "IAudioMetaDataProvider.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

Playlist::Playlist(QString name, QString playlistPath, IAudioMetaDataProvider &metaDataProvider)
: name{ std::move(name) }
, playlistPath{ std::move(playlistPath) }
, audioMetaDataProvider_{ metaDataProvider }
{
}

Playlist::Playlist(QString name, QString playlistPath, std::vector<QUrl> tracks, IAudioMetaDataProvider &metaDataProvider)
: Playlist(std::move(name), std::move(playlistPath), metaDataProvider)
{
    insertTracks(tracks);
}

void Playlist::insertTracks(std::size_t position, std::vector<QUrl> tracksToAdd)
{
    for(const auto &trackUrl : tracksToAdd)
    {
        if(trackUrl.isLocalFile())
        {
            const auto trackPath = trackUrl.path();

            QFileInfo trackFileInfo{ trackPath };
            if(trackFileInfo.isFile())
            {
                tracks.insert(tracks.begin() + position,
                              PlaylistTrack{ trackPath, audioMetaDataProvider_.getMetaData(trackPath) });
                ++position;
            }
            else if(trackFileInfo.isDir())
            {
                const std::function<void(const QString &)> addDirFunc =
                [this, &position, &addDirFunc](const QString &dirPath) {
                    for(const auto &entry :
                        QDir{ dirPath }.entryInfoList(QDir::Files | QDir::AllDirs | QDir::NoDotAndDotDot,
                                                      QDir::DirsFirst))
                    {
                        if(entry.isFile())
                        {
                            const auto trackPath = entry.absoluteFilePath();
                            tracks.insert(tracks.begin() + position,
                                          PlaylistTrack{ trackPath, audioMetaDataProvider_.getMetaData(trackPath) });
                            ++position;
                        }
                        else if(entry.isDir())
                        {
                            addDirFunc(entry.absoluteFilePath());
                        }
                    }
                };

                addDirFunc(trackPath);
            }
        }
        else
        {
            tracks.emplace_back(PlaylistTrack{ trackUrl.toString(), std::nullopt });
        }
    }

    save();
}

void Playlist::insertTracks(std::vector<QUrl> tracksToAdd)
{
    insertTracks(tracks.size(), std::move(tracksToAdd));
}

void Playlist::save()
{
    QFile playlistFile{ playlistPath };
    if(not playlistFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        throw std::runtime_error("Could not save playlist");
    }

    QTextStream ss{ &playlistFile };
    for(const auto &track : tracks)
    {
        ss << track.path << '\n';
    }
}
