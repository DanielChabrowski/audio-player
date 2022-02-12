#pragma once

#include "AudioMetaData.hpp"

#include <taglib/tbytevector.h>

#include <chrono>
#include <optional>

using CoverArt = TagLib::ByteVector;

struct ProvidedMetadata
{
    AudioMetaData audioMetadata;
    std::optional<CoverArt> coverArt;
    std::chrono::seconds lastModified;
};

struct UncachedMetadata
{
    AudioMetaData audioMetadata;
    std::optional<quint64> coverId;
    std::chrono::seconds lastModified;
};
