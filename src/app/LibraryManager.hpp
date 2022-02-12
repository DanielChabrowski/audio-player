#pragma once

#include "Album.hpp"

#include <optional>
#include <vector>

#include <QByteArray>
#include <QPixmap>

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
