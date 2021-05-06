#pragma once

#include "IPlaylistIO.hpp"

class MetaDataCache;
class IAudioMetaDataProvider;

class QString;
class QFileInfo;

class FilesystemPlaylistIO final : public IPlaylistIO
{
public:
    explicit FilesystemPlaylistIO(MetaDataCache &cache, IAudioMetaDataProvider &);

    Playlist load(const QString &filepath) override;
    bool save(const Playlist &) override;
    bool rename(const Playlist &, const QString &newName) override;

    std::vector<PlaylistTrack> loadTracks(const std::vector<QUrl> &) override;

private:
    bool isSupportedFileType(const QFileInfo &fileInfo);

private:
    MetaDataCache &cache_;
    IAudioMetaDataProvider &audioMetaDataProvider_;
};
