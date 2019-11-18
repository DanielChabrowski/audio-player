#include "PlaylistModel.hpp"

#include <QFileInfo>
#include <QSize>

#include "Playlist.hpp"

namespace
{
const char *const labels[] = {
    "", "Artist/album", "Track", "Title", "Duration",
};
} // namespace

PlaylistModel::PlaylistModel(Playlist &playlist, QObject *parent)
: QAbstractListModel{ parent }
, playlist_{ playlist }
{
}

int PlaylistModel::rowCount(const QModelIndex &) const
{
    return static_cast<int>(playlist_.tracks.size());
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

    const auto playlistSize = static_cast<int>(playlist_.tracks.size());
    const auto row = index.row();
    if(row >= playlistSize || row < 0)
    {
        return {};
    }
    const auto col = index.column();

    if(Qt::DisplayRole == role)
    {
        const auto &track = playlist_.tracks.at(row);
        const auto &metaData = track.audioMetaData;

        switch(col)
        {
        case PlaylistColumn::NOW_PLAYING:
        {
            return row == playlist_.currentSongIndex ? QString{ '>' } : "";
        }

        case PlaylistColumn::TITLE:
        {
            return dataTitle(track.path, metaData);
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

Qt::ItemFlags PlaylistModel::flags(const QModelIndex &) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled;
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
