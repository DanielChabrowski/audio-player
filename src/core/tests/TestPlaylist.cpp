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
}

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
