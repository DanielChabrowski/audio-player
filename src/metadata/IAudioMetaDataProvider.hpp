#pragma once

#include "ProvidedMetadata.hpp"

#include <QDir>
#include <QString>

#include <optional>

class IAudioMetaDataProvider
{
public:
    virtual ~IAudioMetaDataProvider() = default;
    virtual std::optional<ProvidedMetadata> getMetaData(const QString &filepath) = 0;
    virtual std::optional<CoverArt> readCoverFromDirectory(QDir directory) = 0;
};
