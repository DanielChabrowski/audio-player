#include "Playlist.hpp"

#include "IAudioMetaDataProvider.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

Playlist::Playlist(QString name, QString playlistPath, IAudioMetaDataProvider &metaDataProvider)
: name_{ std::move(name) }
, path_{ std::move(playlistPath) }
, audioMetaDataProvider_{ metaDataProvider }
{
}

Playlist::Playlist(QString name, QString playlistPath, std::vector<QUrl> tracks, IAudioMetaDataProvider &metaDataProvider)
: Playlist(std::move(name), std::move(playlistPath), metaDataProvider)
{
    insertTracks(tracks);
}

const QString &Playlist::getName() const
{
    return name_;
}

const QString &Playlist::getPath() const
{
    return path_;
}

void Playlist::setPlaylistId(std::uint32_t id)
{
    playlistId = id;
}

std::uint32_t Playlist::getPlaylistId() const
{
    return playlistId;
}

std::size_t Playlist::getTrackCount() const
{
    return tracks_.size();
}

const std::vector<PlaylistTrack> &Playlist::getTracks() const
{
    return tracks_;
}

const PlaylistTrack *Playlist::getTrack(std::size_t index) const
{
    if(index >= tracks_.size())
    {
        return nullptr;
    }

    return &tracks_[index];
}

std::optional<std::size_t> Playlist::getNextTrackIndex() const
{
    if(tracks_.empty())
    {
        return std::nullopt;
    }

    std::size_t nextTrackIndex = currentTrackIndex_ + 1;
    if(nextTrackIndex > tracks_.size() - 1)
    {
        nextTrackIndex = 0;
    }

    return nextTrackIndex;
}

std::optional<std::size_t> Playlist::getPreviousTrackIndex() const
{
    if(tracks_.empty())
    {
        return std::nullopt;
    }

    std::size_t prevTrackIndex = tracks_.size();
    if(currentTrackIndex_ > 0)
    {
        prevTrackIndex = currentTrackIndex_ - 1;
    }

    return prevTrackIndex;
}

int Playlist::getCurrentTrackIndex() const
{
    return currentTrackIndex_;
}

void Playlist::setCurrentTrackIndex(std::size_t newIndex)
{
    currentTrackIndex_ = newIndex;
}

void Playlist::insertTracks(std::size_t position, std::vector<QUrl> tracksToAdd)
{
    const int dropPosition = position;

    for(const auto &trackUrl : tracksToAdd)
    {
        if(trackUrl.isLocalFile())
        {
            const auto trackPath = trackUrl.path();

            QFileInfo trackFileInfo{ trackPath };
            if(trackFileInfo.isFile())
            {
                tracks_.insert(tracks_.begin() + position,
                               PlaylistTrack{ trackPath, audioMetaDataProvider_.getMetaData(trackPath) });
                ++position;
            }
            else if(trackFileInfo.isDir())
            {
                const std::function<void(const QString &)> addDirFunc =
                [this, &position, &addDirFunc](const QString &dirPath) {
                    for(const auto &entry :
                        QDir{ dirPath }.entryInfoList(QDir::Files | QDir::AllDirs | QDir::NoDotAndDotDot,
                                                      QDir::DirsFirst))
                    {
                        if(entry.isFile())
                        {
                            const auto trackPath = entry.absoluteFilePath();
                            tracks_.insert(tracks_.begin() + position,
                                           PlaylistTrack{ trackPath, audioMetaDataProvider_.getMetaData(trackPath) });
                            ++position;
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
            tracks_.insert(tracks_.begin() + position, PlaylistTrack{ trackUrl.toString(), std::nullopt });
            ++position;
        }
    }

    if(dropPosition <= currentTrackIndex_)
    {
        const auto elementsAdded = position - dropPosition;
        currentTrackIndex_ += elementsAdded;
    }

    save();
}

void Playlist::insertTracks(std::vector<QUrl> tracksToAdd)
{
    insertTracks(tracks_.size(), std::move(tracksToAdd));
}

void Playlist::moveTracks(std::vector<std::size_t> indexes, std::size_t moveToIndex)
{
    std::sort(indexes.begin(), indexes.end());
    indexes.erase(std::unique(indexes.begin(), indexes.end()), indexes.end());

    std::size_t offset{ 0 };
    for(const auto index : indexes)
    {
        const auto trackPos = tracks_.begin() + index - offset;
        std::rotate(trackPos, trackPos + 1, tracks_.end());
        ++offset;
    }

    const auto dropPos = tracks_.begin() + std::min(moveToIndex, tracks_.size() - offset);
    std::rotate(dropPos, tracks_.end() - offset, tracks_.end());
}

void Playlist::removeTracks(std::vector<std::size_t> indexes)
{
    // Sort positions to remove to compute the offset index after each removal
    std::sort(indexes.begin(), indexes.end());

    const auto currentIndexShift =
    std::count_if(indexes.cbegin(), indexes.cend(),
                  [currentIndex = this->currentTrackIndex_](const int x) { return x < currentIndex; });
    currentTrackIndex_ -= currentIndexShift;

    std::size_t indexShift{ 0 };
    for(const auto &position : indexes)
    {
        tracks_.erase(tracks_.begin() + (position - indexShift++));
    }

    save();
}

void Playlist::save()
{
    QFile playlistFile{ path_ };
    if(not playlistFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        throw std::runtime_error("Could not save playlist");
    }

    QTextStream ss{ &playlistFile };
    for(const auto &track : tracks_)
    {
        ss << track.path << '\n';
    }
}
