#include "Playlist.hpp"

#include "IPlaylistIO.hpp"

Playlist::Playlist(QString name, QString playlistPath, IPlaylistIO &playlistIO)
: name_{ std::move(name) }
, path_{ std::move(playlistPath) }
, playlistIO_{ playlistIO }
{
}

Playlist::Playlist(QString name, QString playlistPath, std::vector<QUrl> tracks, IPlaylistIO &playlistIO)
: Playlist(std::move(name), std::move(playlistPath), playlistIO)
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
    auto loadedTracks = playlistIO_.loadTracks(tracksToAdd);
    const auto tracksAdded = loadedTracks.size();
    tracks_.insert(tracks_.begin() + position, std::make_move_iterator(loadedTracks.begin()),
                   std::make_move_iterator(loadedTracks.end()));

    if(static_cast<int>(position) <= currentTrackIndex_)
    {
        currentTrackIndex_ += tracksAdded;
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

    save();
}

void Playlist::removeTracks(std::vector<std::size_t> indexes)
{
    // Sort positions to remove to compute the offset index after each removal
    std::sort(indexes.begin(), indexes.end());

    const auto currentIndexShift = std::count_if(indexes.cbegin(), indexes.cend(),
                                                 [currentIndex = this->currentTrackIndex_](const int x) {
                                                     return x < currentIndex;
                                                 });
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
    playlistIO_.save(*this);
}