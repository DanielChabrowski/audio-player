#pragma once

#include "AudioMetaData.hpp"

#include <QString>

#include <optional>
#include <vector>

struct PlaylistTrack
{
    QString path;
    std::optional<AudioMetaData> audioMetaData;
};

struct Playlist
{
    QString name;
    QString playlistPath;
    int currentSongIndex;
    std::vector<PlaylistTrack> tracks;
};
