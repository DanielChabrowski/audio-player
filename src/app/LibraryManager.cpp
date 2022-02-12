#include "LibraryManager.hpp"

#include "MetaDataCache.hpp"

LibraryManager::LibraryManager(MetaDataCache &cache)
: cache_{ cache }
{
}

std::vector<Album> LibraryManager::getAlbums()
{
    static auto albums = cache_.getAlbums();
    return albums;
}

std::optional<QPixmap> LibraryManager::getCoverDataById(quint64 id)
{
    if(const auto cachedCover = std::find_if(
           covers_.cbegin(), covers_.cend(), [id](const auto &cover) { return cover.id == id; });
        cachedCover != covers_.cend())
    {
        return cachedCover->data;
    }


    if(auto coverData = cache_.getCoverDataById(id); coverData)
    {
        auto pixmap = QPixmap::fromImage(QImage::fromData(*coverData));
        covers_.push_back({ id, pixmap });
        return pixmap;
    }

    return std::nullopt;
}
