#include "FilesystemPlaylistIO.hpp"

#include "IAudioMetaDataProvider.hpp"
#include "MetaDataCache.hpp"
#include "Playlist.hpp"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QTextStream>
#include <QUrl>

#include <stdexcept>
#include <vector>

QStringList getSupportedAudioFileExtensions()
{
    return QStringList() << "flac"
                         << "ogg"
                         << "mp3"
                         << "wav"
                         << "m4a"
                         << "m4b"
                         << "webm"
                         << "mkv"
                         << "mp4";
}

FilesystemPlaylistIO::FilesystemPlaylistIO(MetaDataCache &cache, IAudioMetaDataProvider &audioMetaDataProvider)
: cache_{ cache }
, audioMetaDataProvider_{ audioMetaDataProvider }
{
}

Playlist FilesystemPlaylistIO::load(const QString &filename)
{
    QFile playlistFile{ filename };
    if(not playlistFile.open(QIODevice::ReadOnly))
    {
        throw std::runtime_error("Playlist file not found");
    }

    std::vector<QUrl> trackUrls;

    QString line;
    QTextStream ss{ &playlistFile };
    while(ss.readLineInto(&line))
    {
        if(not line.isEmpty())
        {
            trackUrls.emplace_back(QUrl::fromUserInput(line));
        }
    }

    QFileInfo playlistFileInfo{ playlistFile };
    return {
        playlistFileInfo.completeBaseName(),
        playlistFileInfo.absoluteFilePath(),
        std::move(trackUrls),
        *this,
    };
}

bool FilesystemPlaylistIO::save(const Playlist &playlist)
{
    QFile playlistFile{ playlist.getPath() };
    if(not playlistFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return false;
    }

    QTextStream ss{ &playlistFile };
    for(const auto &track : playlist.getTracks())
    {
        ss << track.path << '\n';
    }

    return true;
}

bool FilesystemPlaylistIO::rename(const Playlist &playlist, const QString &newName)
{
    const QFileInfo playlistFileInfo{ playlist.getPath() };
    auto playlistDir{ playlistFileInfo.absoluteDir() };
    return playlistDir.rename(playlist.getName(), newName);
}

bool FilesystemPlaylistIO::isSupportedFileType(const QFileInfo &fileInfo)
{
    static auto supportedFileExtensions = getSupportedAudioFileExtensions();

    if(supportedFileExtensions.contains(fileInfo.suffix()))
    {
        return true;
    }

    return false;
}

std::vector<PlaylistTrack> FilesystemPlaylistIO::loadTracks(const std::vector<QUrl> &urls)
{
    std::vector<QString> tracks;
    tracks.reserve(urls.size());

    std::vector<QString> localFiles;
    localFiles.reserve(urls.size());

    for(const auto &trackUrl : urls)
    {
        if(trackUrl.isLocalFile())
        {
            const auto trackPath = trackUrl.toLocalFile();

            QFileInfo trackFileInfo{ trackPath };
            if(trackFileInfo.isFile())
            {
                if(isSupportedFileType(trackFileInfo))
                {
                    auto path = trackFileInfo.absoluteFilePath();
                    tracks.emplace_back(path);
                    localFiles.push_back(std::move(path));
                }
            }
            else if(trackFileInfo.isDir())
            {
                const std::function<void(const QString &)> addDirFunc = [&](const QString &dirPath)
                {
                    const auto &directoryEntries = QDir{ dirPath }.entryInfoList(
                        QDir::Files | QDir::AllDirs | QDir::NoDotAndDotDot, QDir::DirsFirst);

                    for(const auto &entry : directoryEntries)
                    {
                        if(entry.isFile())
                        {
                            if(isSupportedFileType(entry))
                            {
                                auto path = entry.absoluteFilePath();
                                tracks.emplace_back(path);
                                localFiles.push_back(std::move(path));
                            }
                        }
                        else if(entry.isDir())
                        {
                            addDirFunc(entry.absoluteFilePath());
                        }
                    }
                };

                addDirFunc(trackPath);
            }
        }
        else
        {
            tracks.emplace_back(trackUrl.toString());
        }
    }

    std::vector<PlaylistTrack> playlistTracks;
    playlistTracks.reserve(tracks.size());

    // Keep track of a list of values that are not in cache
    // Use it for faster lookup of duplicates and batch insert into cache database
    std::unordered_map<QString, AudioMetaData> uncached;

    std::set<QString> uniqueLocalFiles{ std::make_move_iterator(localFiles.begin()),
        std::make_move_iterator(localFiles.end()) };
    auto cached = cache_.batchFindByPath(std::move(uniqueLocalFiles));

    std::size_t tempCacheHits{ 0 }, cacheHits{ 0 }, cacheMisses{ 0 };

    for(auto &path : tracks)
    {
        auto cachedValue = cached.find(path);
        if(cachedValue != cached.end())
        {
            ++cacheHits;
            playlistTracks.emplace_back(PlaylistTrack{ std::move(path), cachedValue->second });
            continue;
        }

        auto uncachedValue = uncached.find(path);
        if(uncachedValue != uncached.end())
        {
            ++tempCacheHits;
            playlistTracks.emplace_back(PlaylistTrack{ std::move(path), uncachedValue->second });
            continue;
        }

        ++cacheMisses;

        // NOTE: Called for remote URLs even though we have no chance of retrieving it here
        auto metadata = audioMetaDataProvider_.getMetaData(path);
        if(metadata)
        {
            uncached.insert({ path, *metadata });
            playlistTracks.emplace_back(PlaylistTrack{ std::move(path), std::move(metadata) });
        }
        else
        {
            playlistTracks.emplace_back(PlaylistTrack{ std::move(path), std::nullopt });
        }
    }

    qDebug() << tempCacheHits << "temporary cache hits," << cacheHits << "cache hits,"
             << cacheMisses << "cache misses";

    if(!cache_.cache(std::move(uncached)))
    {
        qWarning() << "Caching audio metadata failed";
    }

    return playlistTracks;
}
