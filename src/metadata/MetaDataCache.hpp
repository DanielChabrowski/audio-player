#pragma once

#include "AudioMetaData.hpp"

#include <QString>

#include <memory>
#include <optional>
#include <set>
#include <unordered_map>

class MetaDataCache
{
public:
    explicit MetaDataCache(QString databaseFile);
    ~MetaDataCache();

    MetaDataCache(const MetaDataCache &) = delete;
    MetaDataCache(MetaDataCache &&) = delete;
    MetaDataCache &operator=(const MetaDataCache &) = delete;
    MetaDataCache &operator=(MetaDataCache &&) = delete;

    std::optional<AudioMetaData> findByPath(const QString &path);
    std::unordered_map<QString, std::optional<AudioMetaData>> batchFindByPath(std::set<QString> paths);

    bool cache(const std::unordered_map<QString, AudioMetaData> &entries);

private:
    void createTable();

private:
    class Impl;
    std::unique_ptr<Impl> impl;
};
