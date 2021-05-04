#include "FilesystemPlaylistIO.hpp"

#include "IAudioMetaDataProvider.hpp"
#include "Playlist.hpp"

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

FilesystemPlaylistIO::FilesystemPlaylistIO(IAudioMetaDataProvider &audioMetaDataProvider)
: audioMetaDataProvider_{ audioMetaDataProvider }
{
}

Playlist FilesystemPlaylistIO::load(const QString &filename)
{
    QFile playlistFile{ filename };
    if(not playlistFile.open(QIODevice::ReadOnly))
    {
        throw std::runtime_error("Playlist file not found");
    }

    std::vector<QUrl> audioFilePaths;

    QString line;
    QTextStream ss{ &playlistFile };
    while(ss.readLineInto(&line))
    {
        if(not line.isEmpty())
        {
            audioFilePaths.emplace_back(QUrl::fromUserInput(line));
        }
    }

    QFileInfo playlistFileInfo{ playlistFile };
    return {
        playlistFileInfo.completeBaseName(),
        playlistFileInfo.absoluteFilePath(),
        audioFilePaths,
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

void FilesystemPlaylistIO::loadTrack(std::vector<PlaylistTrack> &tracks, const QFileInfo &fileInfo)
{
    static auto supportedFileExtensions = getSupportedAudioFileExtensions();

    if(supportedFileExtensions.contains(fileInfo.suffix()))
    {
        const auto trackPath = fileInfo.absoluteFilePath();
        const auto metadata = audioMetaDataProvider_.getMetaData(trackPath);
        tracks.emplace_back(PlaylistTrack{ std::move(trackPath), std::move(metadata) });
    }
}

std::vector<PlaylistTrack> FilesystemPlaylistIO::loadTracks(const std::vector<QUrl> &urls)
{
    std::vector<PlaylistTrack> tracks;

    for(const auto &trackUrl : urls)
    {
        if(trackUrl.isLocalFile())
        {
            const auto trackPath = trackUrl.toLocalFile();

            QFileInfo trackFileInfo{ trackPath };
            if(trackFileInfo.isFile())
            {
                loadTrack(tracks, trackFileInfo);
            }
            else if(trackFileInfo.isDir())
            {
                const std::function<void(const QString &)> addDirFunc = [&](const QString &dirPath) {
                    for(const auto &entry :
                        QDir{ dirPath }.entryInfoList(QDir::Files | QDir::AllDirs | QDir::NoDotAndDotDot,
                                                      QDir::DirsFirst))
                    {
                        if(entry.isFile())
                        {
                            loadTrack(tracks, entry);
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
            tracks.emplace_back(PlaylistTrack{ trackUrl.toString(), std::nullopt });
        }
    }

    return tracks;
}
