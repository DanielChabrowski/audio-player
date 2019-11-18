#pragma once

#include "Playlist.hpp"

class QString;

class PlaylistLoader
{
public:
    Playlist loadFromFile(const QString &filepath);
};
