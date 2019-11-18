#pragma once

#include "Playlist.hpp"

class IAudioMetaDataProvider;

class QString;

class PlaylistLoader
{
public:
    explicit PlaylistLoader(IAudioMetaDataProvider &);
    Playlist loadFromFile(const QString &filepath);

private:
    IAudioMetaDataProvider &audioMetaDataProvider_;
};
