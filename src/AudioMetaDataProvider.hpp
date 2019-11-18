#pragma once

#include "IAudioMetaDataProvider.hpp"

class AudioMetaDataProvider final : public IAudioMetaDataProvider
{
public:
    ~AudioMetaDataProvider();
    AudioMetaData getMetaData(const QString &filepath) override;
};
