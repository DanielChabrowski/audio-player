#include "AlbumModel.hpp"

#include "LibraryManager.hpp"

#include <QColor>
#include <QPixmap>

AlbumModel::AlbumModel(LibraryManager &libraryManager, QObject *parent)
: QAbstractListModel{ parent }
, libraryManager_{ libraryManager }
{
}

int AlbumModel::rowCount(const QModelIndex &) const
{
    return libraryManager_.getAlbums().size();
}

int AlbumModel::columnCount(const QModelIndex &) const
{
    return 1;
}

QVariant AlbumModel::headerData(int section, Qt::Orientation, int role) const
{
    (void)section;
    (void)role;
    return {};
}

QVariant AlbumModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
    {
        return {};
    }

    const auto albums = libraryManager_.getAlbums();

    if(Qt::DisplayRole == role)
    {
        const auto album = albums.at(index.row());

        const auto albumName = album.albumName.isEmpty() ? "Various albums" : album.albumName;
        return QString{ "%1\n%2 tracks" }.arg(albumName).arg(album.trackCount);
    }
    else if(Qt::DecorationRole == role)
    {
        const auto &coverIds = albums.at(index.row()).coverIds;

        if(coverIds.isEmpty())
        {
            return {};
        }

        const auto firstCoverId = coverIds.split(',').first().toULongLong();

        if(auto coverData = libraryManager_.getCoverDataById(firstCoverId); coverData)
        {
            return *coverData;
        }

        return {};
    }

    return {};
}
