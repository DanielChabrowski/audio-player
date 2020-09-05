#include "PlaylistModel.hpp"

#include "Playlist.hpp"

#include <QDataStream>
#include <QDebug>
#include <QFileInfo>
#include <QMimeData>
#include <QSize>
#include <QUrl>

#include <array>
#include <memory>

namespace
{
const std::array<const char *, 5> labels = {
    "", "Artist/album", "Track", "Title", "Duration",
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
    return static_cast<int>(playlist_.getTrackCount());
}

int PlaylistModel::columnCount(const QModelIndex &) const
{
    return 5;
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

    if(Qt::TextAlignmentRole == role)
    {
        return roleAlignment(col);
    }

    return {};
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
        const auto &filepaths = mimeData->urls();
        playlist_.insertTracks(beginRow, std::vector<QUrl>{ filepaths.cbegin(), filepaths.cend() });
    }
    else if(mimeData->hasFormat(playlistIndexesMimeType))
    {
        auto itemsToMove = decodePlaylistIndexesMimeData(*mimeData);
        playlist_.moveTracks(std::move(itemsToMove), beginRow);
    }
    else
    {
        qWarning() << "Unrecognized drop mime data";
        return false;
    }

    update();
    return true;
}

QVariant PlaylistModel::roleAlignment(int column) const
{
    switch(column)
    {
    case PlaylistColumn::NOW_PLAYING:
        return Qt::AlignmentFlag::AlignCenter + Qt::AlignmentFlag::AlignVCenter;

    case PlaylistColumn::TRACK:
    case PlaylistColumn::DURATION:
        return Qt::AlignmentFlag::AlignRight + Qt::AlignVCenter;
    }

    return Qt::AlignmentFlag::AlignLeft + Qt::AlignVCenter;
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
    const QString album{ metaData->albumData.name.isEmpty() ? "?" : metaData->albumData.name };
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
    if(missingData != metaData->albumData.trackNumber)
    {
        const auto track = QString{ "%1" }.arg(metaData->albumData.trackNumber, 2, 10, QChar('0'));

        if(missingData != metaData->albumData.discNumber)
        {
            return QString{ "%1.%2" }.arg(metaData->albumData.discNumber).arg(track);
        }

        return track;
    }

    return {};
}

void PlaylistModel::update()
{
    beginResetModel();
    endResetModel();
}
