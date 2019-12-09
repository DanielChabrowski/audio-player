#include "PlaylistManager.hpp"

#include "FileUtilities.hpp"
#include "IPlaylistIO.hpp"

#include <QDir>
#include <QFile>

PlaylistManager::PlaylistManager(IPlaylistIO &playlistIO, QString playlistDirectory)
: playlistIO_{ playlistIO }
, playlistDirectory_{ std::move(playlistDirectory) }
{
    loadFromDirectory();

    if(playlists_.size() == 0)
    {
        create("Default");
    }
}

std::optional<std::uint32_t> PlaylistManager::add(const QString &filepath)
try
{
    auto playlist = playlistIO_.load(filepath);

    const auto newPlaylistIndex = lastPlaylistIndex_++;
    playlist.setPlaylistId(newPlaylistIndex);
    playlists_.emplace(newPlaylistIndex, std::move(playlist));
    return newPlaylistIndex;
}
catch(const std::runtime_error &)
{
    return std::nullopt;
}

std::optional<std::uint32_t> PlaylistManager::create(const QString &name)
{
    const auto filepath = createPlaylistFile(name);
    if(filepath.isEmpty())
    {
        return std::nullopt;
    }
    return add(filepath);
}

void PlaylistManager::removeById(std::uint32_t id)
{
    auto it = playlists_.find(id);
    if(it == playlists_.end())
    {
        return;
    }

    QFile::remove(it->second.getPath());
    playlists_.erase(id);
}

void PlaylistManager::removeByName(const QString &name)
{
    for(const auto &[key, playlist] : playlists_)
    {
        if(playlist.getName() == name)
        {
            const auto &playlistPath = playlist.getPath();
            QFile::remove(playlistPath);
            playlists_.erase(key);
            return;
        }
    }
}

Playlist *PlaylistManager::get(std::uint32_t id)
{
    auto it = playlists_.find(id);
    return it != playlists_.end() ? &it->second : nullptr;
}

std::unordered_map<std::uint32_t, Playlist> &PlaylistManager::getAll()
{
    return playlists_;
}

QString PlaylistManager::createPlaylistFile(const QString &playlistName)
{
    QDir playlistDir{ playlistDirectory_ };
    if(not playlistDir.exists() and not playlistDir.mkpath(playlistDirectory_))
    {
        return {};
    }

    QFile playlistFile{ getUniqueFilename(playlistDir.absolutePath() + '/' + playlistName) };
    if(not playlistFile.open(QIODevice::ReadWrite))
    {
        return {};
    }

    return playlistFile.fileName();
}

void PlaylistManager::loadFromDirectory()
{
    QDir playlistDir{ playlistDirectory_ };
    if(not playlistDir.exists() and not playlistDir.mkpath(playlistDirectory_))
    {
        // TODO: Error handling
        return;
    }

    for(const auto &entry : playlistDir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot))
    {
        add(entry.absoluteFilePath());
    }
}