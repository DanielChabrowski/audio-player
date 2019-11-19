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

class Playlist
{
public:
    QString name;
    QString playlistPath;
    int currentSongIndex;
    std::vector<PlaylistTrack> tracks;

public:
    void save();
};
