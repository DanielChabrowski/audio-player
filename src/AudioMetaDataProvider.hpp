#pragma once

#include "IAudioMetaDataProvider.hpp"

class AudioMetaDataProvider final : public IAudioMetaDataProvider
{
public:
    ~AudioMetaDataProvider();
    std::optional<AudioMetaData> getMetaData(const QString &filepath) override;
};
