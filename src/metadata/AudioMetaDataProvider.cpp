#include "AudioMetaDataProvider.hpp"

#include <taglib/attachedpictureframe.h>
#include <taglib/fileref.h>
#include <taglib/flacfile.h>
#include <taglib/flacpicture.h>
#include <taglib/id3v2tag.h>
#include <taglib/mp4file.h>
#include <taglib/mpegfile.h>
#include <taglib/tag.h>
#include <taglib/tbytevector.h>
#include <taglib/tpropertymap.h>

#include <QByteArray>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>

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

std::optional<CoverArt> extractFromDirectory(QDir directory)
{
    static QStringList extensions = { "*.jpg", "*.png", "*.jpeg" };

    const auto entries = directory.entryList(
        extensions, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDir::SortFlag::Name);
    if(entries.isEmpty())
    {
        return {};
    }

    QFile imageFile{ directory.absoluteFilePath(entries.first()) };
    if(not imageFile.open(QIODevice::ReadOnly))
    {
        return {};
    }

    const auto imageData = imageFile.readAll();
    return CoverArt{ imageData.data(), static_cast<unsigned int>(imageData.size()) };
}

AudioMetaDataProvider::~AudioMetaDataProvider() = default;

std::optional<ProvidedMetadata> AudioMetaDataProvider::getMetaData(const QString &filepath)
{
    TagLib::FileRef ref{ filepath.toStdString().c_str() };
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
