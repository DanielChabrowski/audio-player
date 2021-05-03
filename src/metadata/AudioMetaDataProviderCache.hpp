#pragma once

#include "IAudioMetaDataProvider.hpp"
#include <QString>
#include <memory>

class AudioMetaDataProviderCache final : public IAudioMetaDataProvider
{
public:
    AudioMetaDataProviderCache(QString databaseFile, IAudioMetaDataProvider *);
    ~AudioMetaDataProviderCache() noexcept;

    std::optional<AudioMetaData> getMetaData(const QString &filepath) override;

private:
    void createTable();
    void cacheEntry(const QString &path, const AudioMetaData &);

private:
    class Impl;
    std::unique_ptr<Impl> impl;
};
