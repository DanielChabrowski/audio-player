#include "MetaDataCache.hpp"

#include <QDebug>
#include <QVariant>
#include <QtSql>

#include <cstdint>
#include <exception>

class MetaDataCache::Impl
{
public:
    Impl(QSqlDatabase database)
    : database{ database }
    {
    }

    Impl(const Impl &) = delete;
    const Impl &operator=(const Impl &) = delete;

    QSqlDatabase database;
};


MetaDataCache::MetaDataCache(QString databaseFile)
{
    qDebug() << "Opening cache database" << databaseFile;

    auto database = QSqlDatabase::addDatabase("QSQLITE");
    database.setDatabaseName(databaseFile);

    if(!database.open())
    {
        throw std::runtime_error("Audio metadata cache database could not be opened");
    }

    createTable();

    impl = std::make_unique<Impl>(std::move(database));
}

MetaDataCache::~MetaDataCache() = default;

std::unordered_map<QString, std::optional<Metadata>> MetaDataCache::batchFindByPath(std::set<QString> paths)
{
    if(!impl->database.transaction())
    {
        qWarning() << "Cannot begin an batch find transaction";
        return {};
    }

    std::unordered_map<QString, std::optional<Metadata>> cachedMetadata;

    QSqlQuery query;
    query.setForwardOnly(true);
    query.prepare("SELECT title, artist, albumName, albumDiscNumber, albumTrackNumber, "
                  "duration, coverId, lastModified FROM metadata WHERE path = ? LIMIT 1");

    for(const auto &path : paths)
    {
        query.addBindValue(path);

        if(query.exec())
        {
            if(query.first())
            {
                QString title = query.value(0).toString();
                QString artist = query.value(1).toString();
                QString albumName = query.value(2).toString();
                int albumDiscNumber = query.value(3).toInt();
                int albumTrackNumber = query.value(4).toInt();
                int duration = query.value(5).toInt();
                std::uint64_t coverId = query.value(6).toULongLong();
                // int lastModified = query.value(7).toInt();

                cachedMetadata.insert({
                    std::move(path),
                    Metadata{
                        AudioMetaData{
                            std::move(title),
                            std::move(artist),
                            std::move(albumName),
                            albumDiscNumber,
                            albumTrackNumber,
                            std::chrono::seconds(duration),
                        },
                        coverId,
                    },
                });
                continue;
            }
        }
        else
        {
            qWarning() << "Cache lookup failed for" << path << query.lastError();
        }
    }

    if(!impl->database.commit())
    {
        qWarning() << "Commit failed, rolling back";
        if(!impl->database.rollback())
        {
            qWarning() << "Rollback failed";
        }
    }

    return cachedMetadata;
}

std::vector<CachedCoverHash> MetaDataCache::getCoverArtHashCache()
{
    QSqlQuery query;
    query.setForwardOnly(true);
    query.prepare("SELECT id, hash from covers");

    if(!query.exec())
    {
        qWarning() << "Could not retrieve cover cache" << query.lastError();
        return {};
    }

    std::vector<CachedCoverHash> covers;
    covers.reserve(query.size() + 1);

    while(query.next())
    {
        covers.emplace_back(CachedCoverHash{
            query.value(0).toULongLong(),
            query.value(1).toByteArray(),
        });
    }

    return covers;
}

std::optional<uint64_t> MetaDataCache::cache(const QByteArray &data, const QByteArray &hash)
{
    QSqlQuery query;
    query.setForwardOnly(true);
    query.prepare("INSERT INTO covers (data, hash) VALUES (?, ?)");

    query.addBindValue(data);
    query.addBindValue(hash);

    if(!query.exec())
    {
        qWarning() << "Could not cache entry" << query.lastError();
        return std::nullopt;
    }

    return query.lastInsertId().toULongLong();
}

bool MetaDataCache::cache(const std::unordered_map<QString, UncachedMetadata> &entries)
{
    if(!impl->database.transaction())
    {
        qWarning() << "Cannot begin an insert transaction";
        return false;
    }

    QSqlQuery query;
    query.prepare(R"(
INSERT INTO metadata (path, title, artist, albumName, albumDiscNumber, albumTrackNumber, duration, coverId, lastModified)
VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
)");

    for(const auto &it : entries)
    {
        query.addBindValue(it.first);
        query.addBindValue(it.second.audioMetadata.title);
        query.addBindValue(it.second.audioMetadata.artist);
        query.addBindValue(it.second.audioMetadata.albumName);
        query.addBindValue(it.second.audioMetadata.discNumber);
        query.addBindValue(it.second.audioMetadata.trackNumber);
        query.addBindValue(static_cast<quint64>(it.second.audioMetadata.duration.count()));
        query.addBindValue(it.second.coverId ? *it.second.coverId : QVariant{ QVariant::ULongLong });
        query.addBindValue(static_cast<quint64>(it.second.lastModified.count()));

        if(!query.exec())
        {
            qWarning() << "Could not cache entry" << query.lastError();
        }
    }

    if(!impl->database.commit())
    {
        qWarning() << "Commit failed, rolling back";
        if(!impl->database.rollback())
        {
            qWarning() << "Rollback failed";
        }

        return false;
    }

    return true;
}

std::vector<Album> MetaDataCache::getAlbums()
{
    QSqlQuery query;
    query.prepare(R"(
SELECT albumName, COUNT(), GROUP_CONCAT(DISTINCT coverId) FROM metadata GROUP BY albumName ORDER BY albumName;
)");

    if(!query.exec())
    {
        qWarning() << "Could not query albums:" << query.lastError().databaseText();
        return {};
    }

    std::vector<Album> albums{};
    while(query.next())
    {
        albums.emplace_back(Album{
            query.value(0).toString(),
            query.value(1).toULongLong(),
            query.value(2).toString(),
        });
    }

    return albums;
}

std::optional<QByteArray> MetaDataCache::getCoverDataById(quint64 id)
{
    QSqlQuery query;
    query.prepare(R"(
SELECT data FROM covers WHERE id = ?;
)");

    query.addBindValue(id);

    if(!query.exec() || !query.first())
    {
        qWarning() << "Could not query cover data:" << query.lastError().databaseText();
        return {};
    }

    return query.value(0).toByteArray();
}

void MetaDataCache::createTable()
{
    QSqlQuery query;
    query.prepare(R"(
CREATE TABLE IF NOT EXISTS "metadata" (
    "path" TEXT NOT NULL UNIQUE,
    "title" TEXT,
    "artist" TEXT,
    "albumName" TEXT,
    "albumDiscNumber" INTEGER DEFAULT 0,
    "albumTrackNumber" INTEGER DEFAULT 0,
    "duration" INTEGER,
    "coverId" INTEGER,
    "lastModified" INTEGER NOT NULL,
    PRIMARY KEY("path")
    FOREIGN KEY("coverId") REFERENCES covers (id)
        ON DELETE SET NULL
);
)");

    if(!query.exec())
    {
        qWarning() << "Could not create a metadata table:" << query.lastError().databaseText();
    }

    query.prepare(R"(
CREATE TABLE IF NOT EXISTS "covers" (
    id INTEGER PRIMARY KEY,
    data BLOB NOT NULL,
    hash BINARY(16) NOT NULL UNIQUE
);
)");

    if(!query.exec())
    {
        qWarning() << "Could not create a covers table:" << query.lastError().databaseText();
    }
}
