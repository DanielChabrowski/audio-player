#pragma once

#include "AudioMetaData.hpp"

#include <QString>

#include <optional>

class IAudioMetaDataProvider
{
public:
    virtual ~IAudioMetaDataProvider() = default;
    virtual std::optional<AudioMetaData> getMetaData(const QString &filepath) = 0;
};
