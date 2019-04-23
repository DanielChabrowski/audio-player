#pragma once

#include "AudioPropertyReader.hpp"

class TaglibAudioPropertyReader : public AudioPropertyReader
{
public:
    Song loadSong(const QString &path) override;
};
