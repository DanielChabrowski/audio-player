#pragma once

#include "Album.hpp"
#include "Metadata.hpp"
#include "ProvidedMetadata.hpp"

#include <QByteArray>
#include <QString>

#include <memory>
#include <optional>
#include <set>
#include <unordered_map>

struct CachedCoverHash
{
    uint64_t id;
    QByteArray hash;
};

class MetaDataCache
{
public:
    explicit MetaDataCache(QString databaseFile);
    ~MetaDataCache();

    MetaDataCache(const MetaDataCache &) = delete;
    MetaDataCache(MetaDataCache &&) = delete;
    MetaDataCache &operator=(const MetaDataCache &) = delete;
    MetaDataCache &operator=(MetaDataCache &&) = delete;

    std::unordered_map<QString, std::optional<Metadata>> batchFindByPath(std::set<QString> paths);

    std::vector<CachedCoverHash> getCoverArtHashCache();
    std::optional<uint64_t> cache(const QByteArray &data, const QByteArray &hash);

    bool cache(const std::unordered_map<QString, UncachedMetadata> &entries);

    std::vector<Album> getAlbums();
    std::optional<QByteArray> getCoverDataById(quint64 id);

private:
    void createTable();

private:
    class Impl;
    std::unique_ptr<Impl> impl;
};
