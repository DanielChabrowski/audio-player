#pragma once

#include <string>
#include <vector>

#include "Song.hpp"

struct Playlist
{
    std::string name;
    std::vector<Song> songs;
};
