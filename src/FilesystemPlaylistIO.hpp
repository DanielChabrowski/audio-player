#pragma once

#include "IPlaylistIO.hpp"

class IAudioMetaDataProvider;

class QString;

class FilesystemPlaylistIO final : public IPlaylistIO
{
public:
    explicit FilesystemPlaylistIO(IAudioMetaDataProvider &);

    Playlist load(const QString &filepath) override;
    bool save(const Playlist &) override;

    std::vector<PlaylistTrack> loadTracks(const std::vector<QUrl> &) override;

private:
    IAudioMetaDataProvider &audioMetaDataProvider_;
};
