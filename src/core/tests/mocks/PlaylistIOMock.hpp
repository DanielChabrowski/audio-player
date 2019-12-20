#pragma once

#include "IPlaylistIO.hpp"

#include <gmock/gmock.h>

class PlaylistIOMock : public IPlaylistIO
{
public:
    MOCK_METHOD(Playlist, load, (const QString &), (override));
    MOCK_METHOD(bool, save, (const Playlist &), (override));
    MOCK_METHOD(bool, rename, (const Playlist &, const QString &), (override));
    MOCK_METHOD(std::vector<PlaylistTrack>, loadTracks, (const std::vector<QUrl> &), (override));
};
