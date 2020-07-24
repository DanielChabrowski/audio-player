#pragma once

#include "AudioMetaData.hpp"

#include <QString>
#include <QUrl>

#include <optional>
#include <vector>

class IPlaylistIO;

struct PlaylistTrack
{
    QString path;
    std::optional<AudioMetaData> audioMetaData;
};

enum class PlayMode
{
    Normal,
    Random,
    RepeatTrack,
};

class Playlist
{
public:
    Playlist(QString name, QString playlistPath, IPlaylistIO &);
    Playlist(QString name, QString playlistPath, std::vector<QUrl> tracks, IPlaylistIO &);

    const QString &getName() const;
    const QString &getPath() const;

    // TODO: Should be assigned only once
    void setPlaylistId(std::uint32_t id);
    std::uint32_t getPlaylistId() const;

    std::size_t getTrackCount() const;
    const std::vector<PlaylistTrack> &getTracks() const;
    const PlaylistTrack *getTrack(std::size_t index) const;

    std::optional<std::size_t> getNextTrackIndex(PlayMode playMode = PlayMode::Normal) const;
    std::optional<std::size_t> getPreviousTrackIndex(PlayMode playMode = PlayMode::Normal) const;

    int getCurrentTrackIndex() const;
    void setCurrentTrackIndex(std::size_t newIndex);

    void insertTracks(std::size_t position, std::vector<QUrl>);
    void insertTracks(std::vector<QUrl>);

    void moveTracks(std::vector<std::size_t> indexes, std::size_t moveToIndex);

    void removeTracks(std::vector<std::size_t> indexes);

private:
    void save();
    std::size_t getRandomIndex() const;

private:
    QString name_;
    QString path_;
    IPlaylistIO &playlistIO_;
    std::vector<PlaylistTrack> tracks_;
    int currentTrackIndex_{ -1 };
    std::uint32_t playlistId;

    friend class PlaylistManager;
};
