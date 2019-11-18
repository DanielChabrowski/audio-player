#pragma once

#include <QString>
#include <vector>

#include "Song.hpp"

struct Playlist
{
    QString name;
    QString playlistPath;
    int currentSongIndex;
    std::vector<Song> songs; // TODO: Remove from Playlist
    std::vector<QString> audioFiles;
};
