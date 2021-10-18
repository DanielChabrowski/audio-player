#include "PlaylistFilterModel.hpp"
#include "Playlist.hpp"
#include "PlaylistModel.hpp"

PlaylistFilterModel::PlaylistFilterModel(QObject *parent)
: QSortFilterProxyModel{ parent }
{
    setDynamicSortFilter(true);
}

void PlaylistFilterModel::setFilterQuery(QString query)
{
    this->query = query;
    invalidateFilter();
}

bool PlaylistFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &) const
{
    if(query.isEmpty()) return true;

    const auto playlistModel = qobject_cast<PlaylistModel *>(sourceModel());
    return playlistModel->getPlaylist().matchesFilterQuery(sourceRow, query);
}
