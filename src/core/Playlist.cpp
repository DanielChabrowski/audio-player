#include "Playlist.hpp"

#include "IPlaylistIO.hpp"

#include <QDebug>
#include <QElapsedTimer>

#include <random>

Playlist::Playlist(QString name, QString playlistPath, IPlaylistIO &playlistIO)
: name_{ std::move(name) }
, path_{ std::move(playlistPath) }
, playlistIO_{ playlistIO }
{
}

Playlist::Playlist(QString name, QString playlistPath, const std::vector<QUrl> &tracks, IPlaylistIO &playlistIO)
: Playlist(std::move(name), std::move(playlistPath), playlistIO)
{
    QElapsedTimer timer;
    timer.start();

    constexpr auto autoSave = false;
    insertTracks(tracks, autoSave);

    const auto elapsed = timer.elapsed();
    qDebug() << "Playlist" << name_ << "with" << tracks.size() << "tracks read in" << elapsed << "ms";
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

std::optional<std::size_t> Playlist::getNextTrackIndex(PlayMode playMode) const
{
    if(tracks_.empty())
    {
        return std::nullopt;
    }

    switch(playMode)
    {
    case PlayMode::RepeatPlaylist:
    {
        std::size_t nextTrackIndex = currentTrackIndex_ + 1;
        return nextTrackIndex < tracks_.size() ? nextTrackIndex : 0;
    }
    case PlayMode::Random:
    {
        return getRandomIndex();
    }
    case PlayMode::RepeatTrack:
    {
        return currentTrackIndex_;
    }
    }

    return std::nullopt;
}

std::optional<std::size_t> Playlist::getPreviousTrackIndex(PlayMode playMode) const
{
    if(tracks_.empty())
    {
        return std::nullopt;
    }

    switch(playMode)
    {
    case PlayMode::RepeatPlaylist:
    {
        return currentTrackIndex_ > 0 ? currentTrackIndex_ - 1 : tracks_.size() - 1;
    }
    case PlayMode::Random:
    {
        return getRandomIndex();
    }
    case PlayMode::RepeatTrack:
    {
        return currentTrackIndex_;
    }
    }

    return std::nullopt;
}

int Playlist::getCurrentTrackIndex() const
{
    return currentTrackIndex_;
}

void Playlist::setCurrentTrackIndex(std::size_t newIndex)
{
    currentTrackIndex_ = newIndex;
}

void Playlist::insertTracks(std::size_t position, const std::vector<QUrl> &tracksToAdd, bool autoSave)
{
    auto loadedTracks = playlistIO_.loadTracks(tracksToAdd);
    const auto tracksAdded = loadedTracks.size();
    tracks_.insert(tracks_.begin() + position, std::make_move_iterator(loadedTracks.begin()),
                   std::make_move_iterator(loadedTracks.end()));

    if(static_cast<int>(position) <= currentTrackIndex_)
    {
        currentTrackIndex_ += tracksAdded;
    }

    if(autoSave)
    {
        save();
    }
}

void Playlist::insertTracks(const std::vector<QUrl> &tracksToAdd, bool autoSave)
{
    insertTracks(tracks_.size(), tracksToAdd, autoSave);
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

void Playlist::removeTracks(std::size_t first, std::size_t count)
{
    const auto tracksCount = tracks_.size();
    const auto begin = std::clamp<std::size_t>(first, 0u, tracksCount);
    const auto end = std::clamp<std::size_t>(first + count, begin, tracksCount);

    if(first >= tracksCount || (first + count) > tracksCount)
    {
        qWarning() << "removeTracks out of bounds" << first << first + count;
    }

    if(currentTrackIndex_ > 0 && static_cast<std::size_t>(currentTrackIndex_) > begin)
    {
        currentTrackIndex_ -= std::min(currentTrackIndex_ - begin, count);
    }

    tracks_.erase(tracks_.begin() + begin, tracks_.begin() + end);
    save();
}

void Playlist::removeDuplicates()
{
    std::vector<PlaylistTrack> unique;
    unique.reserve(tracks_.size());

    for(auto &&track : tracks_)
    {
        if(std::find_if(unique.cbegin(), unique.cend(),
                        [&](const auto &i) { return i.path == track.path; }) == unique.cend())
        {
            unique.push_back(std::move(track));
        }
    }

    tracks_ = unique;
    save();
}

void Playlist::save()
{
    playlistIO_.save(*this);
}

std::size_t Playlist::getRandomIndex() const
{
    using DistributionType = std::uniform_int_distribution<std::size_t>;
    static std::mt19937 generator{ std::random_device{}() };
    static DistributionType distribution{};
    return distribution(generator, DistributionType::param_type{ 0, tracks_.size() - 1 });
}
