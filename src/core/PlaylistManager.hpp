#pragma once

#include "Playlist.hpp"

#include <QString>

#include <optional>
#include <unordered_map>

class IPlaylistIO;

class PlaylistManager
{
public:
    using PlaylistContainer = std::unordered_map<std::uint32_t, Playlist>;

    PlaylistManager(IPlaylistIO &, QString playlistDirectory);

    std::optional<std::uint32_t> add(const QString &filepath);
    std::optional<std::uint32_t> create(const QString &name);

    void removeById(std::uint32_t id);
    void removeByName(const QString &name);

    Playlist *get(std::uint32_t id);
    PlaylistContainer &getAll();

private:
    QString createPlaylistFile(const QString &playlistName);
    void loadFromDirectory();

private:
    IPlaylistIO &playlistIO_;
    QString playlistDirectory_;
    std::uint32_t lastPlaylistIndex_{ 0 };
    PlaylistContainer playlists_;
};
