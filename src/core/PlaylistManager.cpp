#include "PlaylistManager.hpp"

#include "FileUtilities.hpp"
#include "IPlaylistIO.hpp"

#include <QDebug>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>

#include <stdexcept>

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

std::optional<PlaylistId> PlaylistManager::add(const QString &filepath)
try
{
    auto playlist = playlistIO_.load(filepath);

    const auto newPlaylistIndex = PlaylistId{ lastPlaylistIndex_++ };
    playlist.setPlaylistId(newPlaylistIndex);
    playlists_.emplace(newPlaylistIndex, std::move(playlist));
    return newPlaylistIndex;
}
catch(const std::runtime_error &)
{
    return std::nullopt;
}

std::optional<PlaylistId> PlaylistManager::create(const QString &name)
{
    const auto filepath = createPlaylistFile(name);
    if(filepath.isEmpty())
    {
        return std::nullopt;
    }
    return add(filepath);
}

void PlaylistManager::removeById(PlaylistId id)
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

bool PlaylistManager::rename(PlaylistId id, const QString &newName)
{
    auto *playlist = get(id);
    if(not playlist)
    {
        return false;
    }

    const bool hasRenamed = playlistIO_.rename(*playlist, newName);
    if(hasRenamed)
    {
        playlist->name_ = newName;
        playlist->path_ = createPlaylistPath(newName);
    }
    return hasRenamed;
}

Playlist *PlaylistManager::get(PlaylistId id)
{
    auto it = playlists_.find(id);
    return it != playlists_.end() ? &it->second : nullptr;
}

PlaylistManager::PlaylistContainer &PlaylistManager::getAll()
{
    return playlists_;
}

QString PlaylistManager::createPlaylistPath(const QString &playlistName)
{
    return QDir{ playlistDirectory_ }.absoluteFilePath(playlistName);
}

QString PlaylistManager::createPlaylistFile(const QString &playlistName)
{
    QDir playlistDir{ playlistDirectory_ };
    if(not playlistDir.exists() and not playlistDir.mkpath(playlistDirectory_))
    {
        return {};
    }

    QFile playlistFile{ getUniqueFilename(createPlaylistPath(playlistName)) };
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

    const auto playlists = playlistDir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
    qDebug() << "Playlist files found:" << playlists.count();

    QElapsedTimer timer;
    timer.start();

    for(const auto &entry : playlists)
    {
        add(entry.absoluteFilePath());
    }

    const auto elapsed = timer.elapsed();
    qDebug() << "Loaded playlists in" << elapsed << "ms";
}
