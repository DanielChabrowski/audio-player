#include <gtest/gtest.h>

#include "Playlist.hpp"
#include "mocks/PlaylistIOMock.hpp"

#include <QString>
#include <QUrl>

#include <vector>

using namespace ::testing;

namespace
{
const PlaylistTrack trackWithoutMetadata{ "TrackPath", std::nullopt };

std::vector<PlaylistTrack> createTracks(std::size_t count)
{
    std::vector<PlaylistTrack> newTracks;
    for(std::size_t i = 0; i < count; ++i)
    {
        newTracks.emplace_back(PlaylistTrack{ QString{ "NewTrack%1" }.arg(i), std::nullopt });
    }
    return newTracks;
}
} // namespace

struct PlaylistTests : Test
{
    StrictMock<PlaylistIOMock> playlistIOMock{};
};

TEST_F(PlaylistTests, insertTracks)
{
    InSequence s{};

    Playlist playlist{ "TestName", "TestPath", playlistIOMock };
    const std::vector<QUrl> tracksToAdd{ QUrl{ "Track1" }, QUrl{ "Track2" } };
    const auto tracksToAddCount{ tracksToAdd.size() };
    const std::vector<PlaylistTrack> loadedTracks{ trackWithoutMetadata, trackWithoutMetadata };

    EXPECT_EQ(0, playlist.getTrackCount());

    EXPECT_CALL(playlistIOMock, loadTracks(SizeIs(tracksToAddCount))).WillOnce(Return(loadedTracks));
    EXPECT_CALL(playlistIOMock, save);

    playlist.insertTracks(tracksToAdd);

    EXPECT_EQ(tracksToAddCount, playlist.getTrackCount());
}

TEST_F(PlaylistTests, insertTracksAtPosition)
{
    const std::vector<QUrl> tracksToLoad{ QUrl{ "Track1" }, QUrl{ "Track2" }, QUrl{ "Track3" } };

    const PlaylistTrack newTrack1{ "NewTrack1", std::nullopt };
    const PlaylistTrack newTrack2{ "NewTrack2", std::nullopt };
    const PlaylistTrack newTrack3{ "NewTrack3", std::nullopt };
    const std::vector<PlaylistTrack> newTracks{ newTrack1, newTrack2, newTrack3 };
    const auto newTracksCount{ newTracks.size() };

    Playlist playlist{ "TestName", "TestPath", playlistIOMock };

    EXPECT_CALL(playlistIOMock, loadTracks(SizeIs(tracksToLoad.size()))).Times(2).WillRepeatedly(Return(newTracks));
    EXPECT_CALL(playlistIOMock, save).Times(2).WillRepeatedly(Return(true));

    playlist.insertTracks(0, tracksToLoad);
    EXPECT_EQ(newTracksCount, playlist.getTrackCount());

    playlist.insertTracks(1, tracksToLoad);
    ASSERT_EQ(newTracksCount * 2, playlist.getTrackCount());

    const auto &tracks = playlist.getTracks();
    EXPECT_EQ("NewTrack1", tracks.at(0).path);
    EXPECT_EQ("NewTrack1", tracks.at(1).path);
    EXPECT_EQ("NewTrack2", tracks.at(2).path);
    EXPECT_EQ("NewTrack3", tracks.at(3).path);
    EXPECT_EQ("NewTrack2", tracks.at(4).path);
    EXPECT_EQ("NewTrack3", tracks.at(5).path);
}

TEST_F(PlaylistTests, removeTracks)
{
    const std::vector<QUrl> tracksToLoad{};
    const auto newTracksCount{ 5 };
    const auto newTracks = createTracks(newTracksCount);

    EXPECT_CALL(playlistIOMock, loadTracks).WillOnce(Return(newTracks));
    EXPECT_CALL(playlistIOMock, save).Times(2).WillRepeatedly(Return(true));

    Playlist playlist{ "TestName", "TestPath", tracksToLoad, playlistIOMock };
    ASSERT_EQ(newTracksCount, playlist.getTrackCount());

    const std::vector<std::size_t> indexesToRemove{ 0, 2, 4 };
    playlist.removeTracks(indexesToRemove);
    ASSERT_EQ(newTracksCount - indexesToRemove.size(), playlist.getTrackCount());
    const auto &tracks = playlist.getTracks();
    EXPECT_EQ("NewTrack1", tracks.at(0).path);
    EXPECT_EQ("NewTrack3", tracks.at(1).path);
}
