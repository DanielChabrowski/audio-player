#pragma once

#include "AudioMetaData.hpp"

#include <QString>
#include <QUrl>

#include <optional>
#include <vector>

class IAudioMetaDataProvider;

struct PlaylistTrack
{
    QString path;
    std::optional<AudioMetaData> audioMetaData;
};

class Playlist
{
public:
    Playlist(QString name, QString playlistPath, IAudioMetaDataProvider &);
    Playlist(QString name, QString playlistPath, std::vector<QUrl> tracks, IAudioMetaDataProvider &);

    const QString &getName() const;
    const QString &getPath() const;

    // TODO: Should be assigned only once
    void setPlaylistId(std::uint32_t id);
    std::uint32_t getPlaylistId() const;

    std::size_t getTrackCount() const;
    const std::vector<PlaylistTrack> &getTracks() const;
    const PlaylistTrack *getTrack(std::size_t index) const;
    std::optional<std::size_t> getNextTrackIndex() const;
    std::optional<std::size_t> getPreviousTrackIndex() const;

    int getCurrentTrackIndex() const;
    void setCurrentTrackIndex(std::size_t newIndex);

    void insertTracks(std::size_t position, std::vector<QUrl>);
    void insertTracks(std::vector<QUrl>);

    void removeTracks(std::vector<std::size_t> indexes);

    void save();

private:
    QString name_;
    QString path_;
    IAudioMetaDataProvider &audioMetaDataProvider_;
    std::vector<PlaylistTrack> tracks_;
    int currentTrackIndex_{ -1 };
    std::uint32_t playlistId;
};
