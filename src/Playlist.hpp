#pragma once

#include "AudioMetaData.hpp"

#include <QString>
#include <QUrl>

#include <optional>
#include <vector>

class IAudioMetaDataProvider;

struct PlaylistTrack
{
    QString path;
    std::optional<AudioMetaData> audioMetaData;
};

class Playlist
{
public:
    Playlist(QString name, QString playlistPath, IAudioMetaDataProvider &);
    Playlist(QString name, QString playlistPath, std::vector<QUrl> tracks, IAudioMetaDataProvider &);

    QString name;
    QString playlistPath;
    std::vector<PlaylistTrack> tracks;
    int currentSongIndex{ -1 };

    void insertTracks(std::size_t position, std::vector<QUrl>);
    void insertTracks(std::vector<QUrl>);

    void removeTracks(std::vector<std::size_t> indexes);

    void save();

private:
    IAudioMetaDataProvider &audioMetaDataProvider_;
};
