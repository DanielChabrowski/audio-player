#pragma once

#include "Playlist.hpp"

#include <QString>

#include <optional>
#include <unordered_map>

class IPlaylistIO;

class PlaylistManager final
{
public:
    using PlaylistContainer = std::unordered_map<PlaylistId, Playlist, PlaylistIdHasher>;

    PlaylistManager(IPlaylistIO &, QString playlistDirectory);

    std::optional<PlaylistId> add(const QString &filepath);
    std::optional<PlaylistId> create(const QString &name);

    void removeById(PlaylistId id);
    void removeByName(const QString &name);

    bool rename(PlaylistId id, const QString &newName);

    Playlist *get(PlaylistId id);
    PlaylistContainer &getAll();

private:
    QString createPlaylistPath(const QString &playlistName);
    QString createPlaylistFile(const QString &playlistName);
    void loadFromDirectory();

private:
    IPlaylistIO &playlistIO_;
    QString playlistDirectory_;
    decltype(PlaylistId::value) lastPlaylistIndex_{ 0 };
    PlaylistContainer playlists_;
};
