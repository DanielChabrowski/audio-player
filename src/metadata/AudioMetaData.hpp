#pragma once

#include <QString>

#include <chrono>

struct AudioMetaData
{
    QString title;
    QString artist;
    QString albumName;
    int discNumber;
    int trackNumber;
    std::chrono::seconds duration;
};
