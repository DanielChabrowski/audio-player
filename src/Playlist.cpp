#include "Playlist.hpp"

#include <QFile>
#include <QTextStream>

void Playlist::save()
{
    QFile playlistFile{ playlistPath };
    if(not playlistFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        throw std::runtime_error("Could not save playlist");
    }

    QTextStream ss{ &playlistFile };
    for(const auto &track : tracks)
    {
        ss << track.path;
    }
}
