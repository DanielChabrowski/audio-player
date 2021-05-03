#pragma once

#include "IPlaylistIO.hpp"

class IAudioMetaDataProvider;

class QString;
class QFileInfo;

class FilesystemPlaylistIO final : public IPlaylistIO
{
public:
    explicit FilesystemPlaylistIO(IAudioMetaDataProvider &);

    Playlist load(const QString &filepath) override;
    bool save(const Playlist &) override;
    bool rename(const Playlist &, const QString &newName) override;

    std::vector<PlaylistTrack> loadTracks(const std::vector<QUrl> &) override;

private:
    void loadTrack(std::vector<PlaylistTrack> &tracks, const QFileInfo &fileInfo);

private:
    IAudioMetaDataProvider &audioMetaDataProvider_;
};
