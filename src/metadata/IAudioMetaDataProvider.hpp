#pragma once

#include "ProvidedMetadata.hpp"

#include <QString>

#include <optional>

class IAudioMetaDataProvider
{
public:
    virtual ~IAudioMetaDataProvider() = default;
    virtual std::optional<ProvidedMetadata> getMetaData(const QString &filepath) = 0;
};
