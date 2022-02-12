#pragma once

#include "AudioMetaData.hpp"

#include <optional>

struct Metadata
{
    AudioMetaData audioMetadata;
    std::optional<uint64_t> coverArtId;
};
