#pragma once

#include "Playlist.hpp"

#include <QString>

#include <optional>
#include <unordered_map>

class PlaylistLoader;

class PlaylistManager
{
public:
    PlaylistManager(PlaylistLoader &, QString playlistDirectory);

    std::optional<std::uint32_t> add(const QString &filepath);
    std::optional<std::uint32_t> create(const QString &name);

    Playlist &get(std::uint32_t index);
    std::unordered_map<std::uint32_t, Playlist> &getAll();

private:
    QString createPlaylistFile(const QString &playlistName);
    void loadFromDirectory();

private:
    PlaylistLoader &playlistLoader_;
    QString playlistDirectory_;

    std::uint32_t lastPlaylistIndex_{ 0 };
    std::unordered_map<std::uint32_t, Playlist> playlists_;
};
