#include "AudioMetaDataProvider.hpp"

#include <algorithm>
#include <cstdio>
#include <taglib/attachedpictureframe.h>
#include <taglib/fileref.h>
#include <taglib/flacfile.h>
#include <taglib/flacpicture.h>
#include <taglib/id3v2tag.h>
#include <taglib/mp4file.h>
#include <taglib/mpegfile.h>
#include <taglib/tag.h>
#include <taglib/tbytevector.h>
#include <taglib/tfilestream.h>
#include <taglib/tpropertymap.h>

#include <QByteArray>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QScopeGuard>

namespace
{
void extractId3v2Picture(TagLib::ID3v2::Tag *tag, std::optional<CoverArt> &coverArt)
{
    if(!tag)
    {
        return;
    }

    const auto &pictureFrameList = tag->frameListMap()["APIC"];
    for(const auto frame : pictureFrameList)
    {
        const auto *apicFrame = static_cast<TagLib::ID3v2::AttachedPictureFrame *>(frame);
        if(apicFrame->type() == TagLib::ID3v2::AttachedPictureFrame::FrontCover)
        {
            coverArt.emplace(apicFrame->picture());
            break;
        }
    };
}

void extractFlacPicture(TagLib::FLAC::File *file, std::optional<CoverArt> &coverArt)
{
    const auto &pictureList = file->pictureList();
    for(const auto picture : pictureList)
    {
        if(picture->type() == TagLib::FLAC::Picture::Type::FrontCover)
        {
            coverArt.emplace(picture->data());
            break;
        }
    }
}

void extractMp4Picture(TagLib::MP4::File *file, std::optional<CoverArt> &coverArt)
{
    auto coverItem = file->tag()->item("covr");
    if(coverItem.isValid())
    {
        auto coverItemList = coverItem.toCoverArtList();
        coverArt.emplace(coverItemList.front().data());
    }
}

std::optional<CoverArt> extractCoverArt(TagLib::File *file)
{
    std::optional<CoverArt> coverArt;

    if(auto mpegFile = dynamic_cast<TagLib::MPEG::File *>(file); mpegFile)
    {
        extractId3v2Picture(mpegFile->ID3v2Tag(), coverArt);
    }
    else if(auto flacFile = dynamic_cast<TagLib::FLAC::File *>(file); flacFile)
    {
        extractFlacPicture(flacFile, coverArt);

        if(!coverArt)
        {
            extractId3v2Picture(flacFile->ID3v2Tag(), coverArt);
        }
    }
    else if(auto mp4File = dynamic_cast<TagLib::MP4::File *>(file))
    {
        extractMp4Picture(mp4File, coverArt);
    }

    return coverArt;
}

std::optional<CoverArt> readCoverArt(const QString &path)
{
    // Use c-style IO to preallocate taglib buffer and use it for read directly
    const auto imageFile = std::fopen(path.toStdString().c_str(), "rb");
    if(not imageFile)
    {
        return {};
    }

    const auto guard = qScopeGuard([imageFile] { std::fclose(imageFile); });

    std::fseek(imageFile, 0, SEEK_END);
    const unsigned int fileLength = std::ftell(imageFile);
    std::fseek(imageFile, 0, SEEK_SET);

    auto coverArt = CoverArt(fileLength);

    const auto bytesRead = std::fread(coverArt.data(), sizeof(char), fileLength, imageFile);
    if(bytesRead != fileLength)
    {
        qWarning() << "Couldn't read the whole file";
        return {};
    }

    return coverArt;
}

std::optional<CoverArt> extractFromDirectory(QDir directory)
{
    const auto entries = directory.entryList(QDir::Files | QDir::NoDotAndDotDot, QDir::SortFlag::Unsorted);

    const auto coverFilePath = std::find_if(entries.cbegin(), entries.cend(),
        [](const auto &entry)
        {
            constexpr auto jpgExt = QLatin1String(".jpg");
            constexpr auto pngExt = QLatin1String(".png");
            constexpr auto jpegExt = QLatin1String(".jpeg");

            return entry.endsWith(jpgExt, Qt::CaseInsensitive) ||
                   entry.endsWith(pngExt, Qt::CaseInsensitive) ||
                   entry.endsWith(jpegExt, Qt::CaseInsensitive);
        });

    if(coverFilePath == entries.cend())
    {
        return {};
    }

    const auto absFilePath = directory.absoluteFilePath(*coverFilePath);
    return readCoverArt(absFilePath);
}
} // namespace

AudioMetaDataProvider::~AudioMetaDataProvider() = default;

std::optional<ProvidedMetadata> AudioMetaDataProvider::getMetaData(const QString &filepath)
{
    TagLib::FileStream stream{ filepath.toStdString().c_str(), true };
    TagLib::FileRef ref(&stream);
    if(ref.isNull())
    {
        return {};
    }

    QFileInfo fileInfo{ filepath };
    auto lastModified = fileInfo.lastModified().toUTC().currentSecsSinceEpoch();

    const auto *audioProperties = ref.audioProperties();
    const auto duration = audioProperties ? audioProperties->length() : 0;

    const auto *tags = ref.tag();
    if(not tags)
    {
        return ProvidedMetadata{
            AudioMetaData{
                QString{},
                QString{},
                QString{},
                -1,
                -1,
                std::chrono::seconds{ duration },
            },
            std::nullopt,
            std::chrono::seconds{ lastModified },
        };
    }

    const auto trackNumber = tags->track() > 0 ? static_cast<int>(tags->track()) : -1;

    auto artist = TStringToQString(tags->artist());
    auto discNumber = -1;

    const auto properties = tags->properties();
    for(const auto &property : properties)
    {
        for(const auto &propValue : property.second)
        {
            const auto &propertyName = property.first.to8Bit(true);
            if(u8"DISCNUMBER" == propertyName)
            {
                discNumber = propValue.toInt();
            }
            else if(artist.isEmpty() && u8"ALBUMARTIST" == propertyName)
            {
                artist = TStringToQString(propValue);
            }
        }
    }

    auto coverArt = extractCoverArt(ref.file());
    if(not coverArt)
    {
        coverArt = extractFromDirectory(fileInfo.absoluteDir());
    }

    return ProvidedMetadata{
        AudioMetaData{
            TStringToQString(tags->title()),
            std::move(artist),
            TStringToQString(tags->album()),
            discNumber,
            trackNumber,
            std::chrono::seconds{ duration },

        },
        std::move(coverArt),
        std::chrono::seconds{ lastModified },
    };
}
