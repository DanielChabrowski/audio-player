#include "MetaDataCache.hpp"

#include <QDebug>
#include <QVariant>
#include <QtSql>

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

std::unordered_map<QString, std::optional<AudioMetaData>> MetaDataCache::batchFindByPath(std::set<QString> paths)
{
    if(!impl->database.transaction())
    {
        qWarning() << "Cannot begin an batch find transaction";
        return {};
    }

    std::unordered_map<QString, std::optional<AudioMetaData>> cachedMetadata;

    QSqlQuery query;
    query.prepare("SELECT title, artist, albumName, albumDiscNumber, albumTrackNumber, "
                  "duration FROM metadata WHERE path = ? LIMIT 1");

    for(const auto &path : paths)
    {
        query.addBindValue(path);

        if(query.exec())
        {
            if(query.next())
            {
                QString title = query.value(0).toString();
                QString artist = query.value(1).toString();
                QString albumName = query.value(2).toString();
                int albumDiscNumber = query.value(3).toInt();
                int albumTrackNumber = query.value(4).toInt();
                int duration = query.value(5).toInt();

                cachedMetadata.insert({
                    std::move(path),
                    AudioMetaData{
                        std::move(title),
                        std::move(artist),
                        std::move(albumName),
                        albumDiscNumber,
                        albumTrackNumber,
                        std::chrono::seconds(duration),
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

bool MetaDataCache::cache(const std::unordered_map<QString, AudioMetaData> &entries)
{
    if(!impl->database.transaction())
    {
        qWarning() << "Cannot begin an insert transaction";
        return false;
    }

    QSqlQuery query;
    query.prepare(R"(
INSERT INTO metadata (path, title, artist, albumName, albumDiscNumber, albumTrackNumber, duration)
VALUES (?, ?, ?, ?, ?, ?, ?)
)");


    for(const auto &it : entries)
    {
        query.addBindValue(it.first);
        query.addBindValue(it.second.title);
        query.addBindValue(it.second.artist);
        query.addBindValue(it.second.albumName);
        query.addBindValue(it.second.discNumber);
        query.addBindValue(it.second.trackNumber);
        query.addBindValue(static_cast<qlonglong>(it.second.duration.count()));

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
    PRIMARY KEY("path")
);
)");

    if(!query.exec())
    {
        qWarning() << "Could not create a metadata table:" << query.lastError().databaseText();
    }
}
