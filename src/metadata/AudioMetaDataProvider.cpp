#include "AudioMetaDataProvider.hpp"

#include "taglib/fileref.h"
#include "taglib/tag.h"
#include "taglib/tpropertymap.h"

AudioMetaDataProvider::~AudioMetaDataProvider() = default;

std::optional<AudioMetaData> AudioMetaDataProvider::getMetaData(const QString &filepath)
{
    TagLib::FileRef ref{ filepath.toStdString().c_str() };
    if(ref.isNull())
    {
        return {};
    }

    const auto *audioProperties = ref.audioProperties();
    const auto duration = audioProperties ? audioProperties->length() : 0;

    const auto *tags = ref.tag();
    if(not tags)
    {
        return AudioMetaData{ QString{}, QString{}, QString{}, -1, -1, std::chrono::seconds{ duration } };
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

    return AudioMetaData{
        TStringToQString(tags->title()),
        std::move(artist),
        TStringToQString(tags->album()),
        discNumber,
        trackNumber,
        std::chrono::seconds{ duration },
    };
}
