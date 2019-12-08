#pragma once

#include <vector>

class Playlist;
struct PlaylistTrack;

class QUrl;
class QString;

class IPlaylistIO
{
public:
    virtual ~IPlaylistIO() = default;
    virtual Playlist load(const QString &filepath) = 0;
    virtual bool save(const Playlist &) = 0;

    virtual std::vector<PlaylistTrack> loadTracks(const std::vector<QUrl> &) = 0;
};
