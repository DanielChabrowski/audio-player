#pragma once

#include <string>
#include <vector>

#include "Song.hpp"

struct Playlist
{
    std::string name;
    int currentSongIndex;
    std::vector<Song> songs;
};
