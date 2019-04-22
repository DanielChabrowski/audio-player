#pragma once

#include <QString>
#include <vector>

#include "Song.hpp"

struct Playlist
{
    QString name;
    int currentSongIndex;
    std::vector<Song> songs;
};
