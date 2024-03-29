#include "PlaylistModel.hpp"

#include "Playlist.hpp"

#include <QDataStream>
#include <QFileInfo>
#include <QMimeData>
#include <QUrl>

#include <array>
#include <memory>
#include <vector>

namespace
{
// Keep in check with PlaylistColumn enum
constexpr std::array<const char *, 5> labels = {
    "",
    "Artist/album",
    "Track",
    "Title",
    "Duration",
};

constexpr auto playlistIndexesMimeType{ "application/playlist.indexes" };

std::vector<std::size_t> decodePlaylistIndexesMimeData(const QMimeData &mimeData)
{
    QByteArray encodedData = mimeData.data(playlistIndexesMimeType);
    QDataStream stream(&encodedData, QIODevice::ReadOnly);

    std::vector<std::size_t> indexes;
    while(!stream.atEnd())
    {
        int row{ -1 };
        stream >> row;
        indexes.emplace_back(static_cast<std::size_t>(row));
    }

    // Remove duplicated rows, QModelIndexList contains indexes for all columns
    indexes.erase(std::unique(indexes.begin(), indexes.end()), indexes.end());

    return indexes;
}
} // namespace

PlaylistModel::PlaylistModel(Playlist &playlist, QObject *parent)
: QAbstractListModel{ parent }
, playlist_{ playlist }
{
}

int PlaylistModel::rowCount(const QModelIndex &) const
{
    return static_cast<int>(std::min(playlist_.getTrackCount(), fetched_));
}

int PlaylistModel::columnCount(const QModelIndex &) const
{
    return labels.size();
}

QVariant PlaylistModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(Qt::DisplayRole != role)
    {
        return {};
    }

    if(Qt::Horizontal == orientation)
    {
        return QString(labels[section]);
    }
    else
    {
        return QString("Vertical %1").arg(section);
    }
}

QVariant PlaylistModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
    {
        return {};
    }

    const auto playlistSize = static_cast<int>(playlist_.getTrackCount());
    const auto row = index.row();
    if(row >= playlistSize || row < 0)
    {
        return {};
    }
    const auto col = index.column();
    const auto currentTrackIndex = playlist_.getCurrentTrackIndex();

    if(Qt::DisplayRole == role)
    {
        const auto *track = playlist_.getTrack(row);
        const auto &metaData = track->audioMetaData;

        switch(col)
        {
        case PlaylistColumn::NOW_PLAYING:
        {
            return row == currentTrackIndex ? QString{ '>' } : "";
        }

        case PlaylistColumn::TITLE:
        {
            return dataTitle(track->path, metaData);
        }

        case PlaylistColumn::ARTIST_ALBUM:
        {
            return dataArtistAlbum(metaData);
        }

        case PlaylistColumn::TRACK:
        {
            return dataTrack(metaData);
        }

        case PlaylistColumn::DURATION:
        {
            return dataDuration(metaData);
        }
        }

        return QString("Unsupported column %1").arg(index.column());
    }
    else if(Qt::TextAlignmentRole == role)
    {
        return roleAlignment(col);
    }

    return {};
}

bool PlaylistModel::canFetchMore(const QModelIndex &parent) const
{
    if(parent.isValid()) return false;

    return fetched_ < playlist_.getTrackCount();
}

void PlaylistModel::fetchMore(const QModelIndex &parent)
{
    if(parent.isValid()) return;

    const qint64 remainder = playlist_.getTrackCount() - fetched_;
    const qint64 fetchCount = std::min<qint64>(50, remainder);

    if(fetchCount <= 0) return;

    beginInsertRows(QModelIndex(), fetched_, fetched_ + fetchCount - 1);
    fetched_ += fetchCount;
    endInsertRows();
}

bool PlaylistModel::removeRows(int first, int count, const QModelIndex &parent)
{
    beginRemoveRows(parent, first, first + count - 1);

    playlist_.removeTracks(first, count);
    fetched_ -= count;

    endRemoveRows();

    return true;
}

Qt::ItemFlags PlaylistModel::flags(const QModelIndex &index) const
{
    if(index.isValid())
    {
        return Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled;
    }
    else
    {
        return Qt::ItemIsDropEnabled | Qt::ItemIsEnabled;
    }
}

Qt::DropActions PlaylistModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

QMimeData *PlaylistModel::mimeData(const QModelIndexList &indexes) const
{
    auto mimeData = std::make_unique<QMimeData>();

    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    for(const auto &index : indexes)
    {
        if(index.isValid())
        {
            stream << index.row();
        }
    }

    mimeData->setData(playlistIndexesMimeType, encodedData);
    return mimeData.release();
}

bool PlaylistModel::canDropMimeData(const QMimeData *mimeData, Qt::DropAction, int, int, const QModelIndex &) const
{
    return mimeData->hasUrls() or mimeData->hasFormat(playlistIndexesMimeType);
}

bool PlaylistModel::dropMimeData(const QMimeData *mimeData, Qt::DropAction action, int row, int, const QModelIndex &parent)
{
    if(Qt::IgnoreAction == action)
    {
        return true;
    }

    int beginRow{ 0 };
    if(row != -1)
    {
        beginRow = row;
    }
    else if(parent.isValid())
    {
        beginRow = parent.row();
    }
    else
    {
        beginRow = rowCount(QModelIndex());
    }

    if(mimeData->hasUrls())
    {
        // Cannot beginInsert because the amount of elements added is unknown
        // due to URLs sometimes being directories
        beginResetModel();

        const auto &filepaths = mimeData->urls();
        playlist_.insertTracks(beginRow, std::vector<QUrl>{ filepaths.cbegin(), filepaths.cend() });

        endResetModel();
    }
    else if(mimeData->hasFormat(playlistIndexesMimeType))
    {
        // beginMoveRows could be used here but it would require split into
        // consecutive index views
        beginResetModel();

        auto itemsToMove = decodePlaylistIndexesMimeData(*mimeData);
        playlist_.moveTracks(std::move(itemsToMove), beginRow);

        endResetModel();
    }
    else
    {
        qWarning() << "Unrecognized drop mime data";
        return false;
    }

    return true;
}

QVariant PlaylistModel::roleAlignment(int column) const
{
    switch(column)
    {
    case PlaylistColumn::NOW_PLAYING:
        return Qt::AlignCenter;

    case PlaylistColumn::TRACK:
    case PlaylistColumn::DURATION:
        return Qt::Alignment{ Qt::AlignRight | Qt::AlignVCenter }.toInt();
    }

    return Qt::Alignment{ Qt::AlignLeft | Qt::AlignVCenter }.toInt();
}

QVariant PlaylistModel::dataTitle(const QString &filepath, const std::optional<AudioMetaData> &metaData) const
{
    if(metaData and not metaData->title.isEmpty())
    {
        return metaData->title;
    }

    return QFileInfo(filepath).completeBaseName();
}

QVariant PlaylistModel::dataArtistAlbum(const std::optional<AudioMetaData> &metaData) const
{
    if(not metaData)
    {
        return "? - ?";
    }

    const QString artist{ metaData->artist.isEmpty() ? "?" : metaData->artist };
    const QString album{ metaData->albumName.isEmpty() ? "?" : metaData->albumName };
    return artist + " - " + album;
}

QVariant PlaylistModel::dataDuration(const std::optional<AudioMetaData> &metaData) const
{
    if(not metaData)
    {
        return {};
    }

    const auto duration = metaData->duration.count();
    const auto minutes = duration / 60;
    const auto seconds = duration % 60;
    return QString{ "%1:%2" }.arg(minutes).arg(seconds, 2, 10, QChar('0'));
}

QVariant PlaylistModel::dataTrack(const std::optional<AudioMetaData> &metaData) const
{
    if(not metaData)
    {
        return {};
    }

    constexpr int missingData{ -1 };
    if(missingData != metaData->trackNumber)
    {
        const auto track = QString{ "%1" }.arg(metaData->trackNumber, 2, 10, QChar('0'));

        if(missingData != metaData->discNumber)
        {
            return QString{ "%1.%2" }.arg(metaData->discNumber).arg(track);
        }

        return track;
    }

    return {};
}

void PlaylistModel::onDuplicateRemoveRequest()
{
    beginResetModel();
    playlist_.removeDuplicates();
    endResetModel();
}

void PlaylistModel::onInsertRequest(QStringList filenames)
{
    // Cannot beginInsert because the amount of elements added is unknown
    // due to URLs sometimes being directories
    beginResetModel();

    std::vector<QUrl> filepaths;
    filepaths.reserve(filenames.size());

    std::transform(filenames.begin(), filenames.end(), std::back_inserter(filepaths),
        [](QString filename) { return QUrl::fromUserInput(filename); });

    playlist_.insertTracks(playlist_.getTrackCount(), filepaths);

    endResetModel();
}
