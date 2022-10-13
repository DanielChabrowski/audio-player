#pragma once

#include "Album.hpp"

#include <QByteArray>
#include <QPixmap>

#include <optional>
#include <vector>

class MetaDataCache;

class LibraryManager
{
public:
    explicit LibraryManager(MetaDataCache &);

    LibraryManager(LibraryManager &) = delete;
    LibraryManager &operator=(LibraryManager &) = delete;

    std::vector<Album> getAlbums();
    std::optional<QPixmap> getCoverDataById(quint64 id);

private:
    MetaDataCache &cache_;

    struct StoredCover
    {
        quint64 id;
        QPixmap data;
    };
    std::vector<StoredCover> covers_;
};
