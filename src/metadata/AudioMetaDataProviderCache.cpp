#include "AudioMetaDataProviderCache.hpp"
#include <QDebug>
#include <QVariant>
#include <QtSql>
#include <exception>

class AudioMetaDataProviderCache::Impl
{
public:
    Impl(IAudioMetaDataProvider *provider, QSqlDatabase database)
    : provider{ provider }
    , database{ database }
    {
    }

    Impl(const Impl &) = delete;
    const Impl &operator=(const Impl &) = delete;

    IAudioMetaDataProvider *provider;
    QSqlDatabase database;
};

AudioMetaDataProviderCache::AudioMetaDataProviderCache(QString databaseFile, IAudioMetaDataProvider *provider)
{
    qDebug() << "Opening cache database" << databaseFile;

    auto database = QSqlDatabase::addDatabase("QSQLITE");
    database.setDatabaseName(databaseFile);

    if(!database.open())
    {
        throw std::runtime_error("Audio metadata cache database could not be opened");
    }

    createTable();

    impl = std::make_unique<Impl>(provider, std::move(database));
}

AudioMetaDataProviderCache::~AudioMetaDataProviderCache() noexcept = default;

std::optional<AudioMetaData> AudioMetaDataProviderCache::getMetaData(const QString &filepath)
{
    QSqlQuery query;
    query.prepare(
        "SELECT title, artist, albumName, albumDiscNumber, albumDiscTotal, albumTrackNumber, "
        "albumTrackTotal, duration FROM metadata WHERE path = :path LIMIT 1");
    query.bindValue(":path", filepath);

    if(query.exec())
    {
        if(query.next())
        {
            QString title = query.value(0).toString();
            QString artist = query.value(1).toString();
            QString albumName = query.value(2).toString();
            int albumDiscNumber = query.value(3).toInt();
            int albumDiscTotal = query.value(4).toInt();
            int albumTrackNumber = query.value(5).toInt();
            int albumTrackTotal = query.value(6).toInt();
            int duration = query.value(7).toInt();

            return AudioMetaData{
                title,
                artist,
                AudioAlbumMetaData{ albumName, albumDiscNumber, albumDiscTotal, albumTrackNumber, albumTrackTotal },
                std::chrono::seconds(duration),
            };
        }
    }
    else
    {
        qWarning() << "Cache lookup failed for" << filepath << query.lastError();
    }

    auto metadata = impl->provider->getMetaData(filepath);
    if(metadata.has_value())
    {
        cacheEntry(filepath, *metadata);
    }

    return metadata;
}

void AudioMetaDataProviderCache::createTable()
{
    QSqlQuery query;
    query.prepare(R"(
CREATE TABLE IF NOT EXISTS "metadata" (
    "path" TEXT NOT NULL UNIQUE,
    "title" TEXT,
    "artist" TEXT,
    "albumName" TEXT,
    "albumDiscNumber" INTEGER DEFAULT 0,
    "albumDiscTotal" INTEGER DEFAULT 0,
    "albumTrackNumber" INTEGER DEFAULT 0,
    "albumTrackTotal" INTEGER DEFAULT 0,
    "duration" INTEGER,
    PRIMARY KEY("path")
);
)");

    if(!query.exec())
    {
        qWarning() << "Could not create a metadata table:" << query.lastError().databaseText();
    }
}

void AudioMetaDataProviderCache::cacheEntry(const QString &path, const AudioMetaData &metadata)
{
    QSqlQuery query;
    query.prepare(
        R"(
INSERT INTO metadata (path, title, artist, albumName, albumDiscNumber, albumDiscTotal, albumTrackNumber, albumTrackTotal, duration)
VALUES (:path, :title, :artist, :albumName, :albumDiscNumber, :albumDiscTotal, :albumTrackNumber, :albumTrackTotal, :duration)
)");

    query.bindValue(":path", path);
    query.bindValue(":title", metadata.title);
    query.bindValue(":artist", metadata.artist);
    query.bindValue(":albumName", metadata.albumData.name);
    query.bindValue(":albumDiscNumber", metadata.albumData.discNumber);
    query.bindValue(":albumDiscTotal", metadata.albumData.discTotal);
    query.bindValue(":albumTrackNumber", metadata.albumData.trackNumber);
    query.bindValue(":albumTrackTotal", metadata.albumData.trackTotal);
    query.bindValue(":duration", static_cast<qlonglong>(metadata.duration.count()));

    if(!query.exec())
    {
        qWarning() << "Could not cache entry" << query.lastError();
    }
}
