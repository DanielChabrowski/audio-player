#include "Playlist.hpp"

#include "IAudioMetaDataProvider.hpp"

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

void Playlist::insertTracks(std::size_t, std::vector<QUrl> tracksToAdd)
{
    for(const auto &trackUrl : tracksToAdd)
    {
        if(trackUrl.isLocalFile())
        {
            const auto trackPath = trackUrl.path();

            QFileInfo trackFileInfo{ trackPath };
            if(trackFileInfo.isFile())
            {
                tracks.emplace_back(PlaylistTrack{ trackPath, audioMetaDataProvider_.getMetaData(trackPath) });
            }
            else if(trackFileInfo.isDir())
            {
                // TODO: Adding directory
            }
        }
        else
        {
            tracks.emplace_back(PlaylistTrack{ trackUrl.toString(), std::nullopt });
        }
    }

    save();
}

void Playlist::insertTracks(std::vector<QUrl> tracks)
{
    insertTracks(tracks.size() - 1, std::move(tracks));
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
