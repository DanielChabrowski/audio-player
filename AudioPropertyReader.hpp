#pragma once

#include <QString>

#include "Song.hpp"

class AudioPropertyReader
{
public:
    virtual ~AudioPropertyReader() = default;
    virtual Song loadSong(const QString &path) = 0;
};
