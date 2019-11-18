#pragma once

#include "AudioMetaData.hpp"

#include <QString>

class IAudioMetaDataProvider
{
public:
    virtual ~IAudioMetaDataProvider() = default;
    virtual AudioMetaData getMetaData(const QString &filepath) = 0;
};
