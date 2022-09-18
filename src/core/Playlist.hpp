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
    RepeatPlaylist,
    RepeatTrack,
    Random,
};

struct PlaylistId
{
    std::uint32_t value;
};

inline bool operator==(const PlaylistId &l, const PlaylistId &r) noexcept
{
    return l.value == r.value;
}

struct PlaylistIdHasher
{
    std::size_t operator()(const PlaylistId &id) const noexcept;
};

class Playlist final
{
public:
    Playlist(QString name, QString playlistPath, IPlaylistIO &);
    Playlist(QString name, QString playlistPath, const std::vector<QUrl> &tracks, IPlaylistIO &);

    Playlist(Playlist &&) = default;
    Playlist &operator=(Playlist &&) = delete; // clang: implicitly deleted by PlaylistIO ref

    Playlist(const Playlist &) = delete;
    Playlist &operator=(const Playlist &) = delete;

    const QString &getName() const;
    const QString &getPath() const;

    // TODO: Should be assigned only once
    void setPlaylistId(PlaylistId id);
    PlaylistId getPlaylistId() const;

    std::size_t getTrackCount() const;
    const std::vector<PlaylistTrack> &getTracks() const;
    const PlaylistTrack *getTrack(std::size_t index) const;

    std::optional<std::size_t> getNextTrackIndex(PlayMode playMode = PlayMode::RepeatPlaylist) const;
    std::optional<std::size_t> getPreviousTrackIndex(PlayMode playMode = PlayMode::RepeatPlaylist) const;

    int getCurrentTrackIndex() const;
    void setCurrentTrackIndex(std::size_t newIndex);

    void insertTracks(std::size_t position, const std::vector<QUrl> &, bool autoSave = true);
    void insertTracks(const std::vector<QUrl> &, bool autoSave = true);

    void moveTracks(std::vector<std::size_t> indexes, std::size_t moveToIndex);

    void removeTracks(std::size_t first, std::size_t count);

    void removeDuplicates();

    bool matchesFilterQuery(std::size_t trackIndex, QString query) const;

private:
    void save();
    std::size_t getRandomIndex() const;

private:
    QString name_;
    QString path_;
    IPlaylistIO &playlistIO_;
    std::vector<PlaylistTrack> tracks_;
    int currentTrackIndex_{ -1 };
    PlaylistId playlistId{ 0 };

    friend class PlaylistManager;
};
