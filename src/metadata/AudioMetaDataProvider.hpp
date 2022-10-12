#pragma once

#include "IAudioMetaDataProvider.hpp"

class AudioMetaDataProvider final : public IAudioMetaDataProvider
{
public:
    ~AudioMetaDataProvider();
    std::optional<ProvidedMetadata> getMetaData(const QString &filepath) override;
    std::optional<CoverArt> readCoverFromDirectory(QDir directory) override;
};
