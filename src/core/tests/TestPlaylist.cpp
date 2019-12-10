#include <gtest/gtest.h>

#include "Playlist.hpp"
#include "mocks/PlaylistIOMock.hpp"

#include <QString>
#include <QUrl>

#include <vector>

using namespace ::testing;

namespace
{
std::vector<PlaylistTrack> createTracks(std::size_t count)
{
    std::vector<PlaylistTrack> newTracks;
    for(std::size_t i = 0; i < count; ++i)
    {
        newTracks.emplace_back(PlaylistTrack{ QString{ "NewTrack%1" }.arg(i), std::nullopt });
    }
    return newTracks;
}

void validateTracks(const Playlist &playlist, std::vector<QString> trackPaths)
{
    const auto &tracks = playlist.getTracks();
    ASSERT_EQ(trackPaths.size(), tracks.size());

    for(std::size_t i = 0; i < tracks.size(); ++i)
    {
        EXPECT_EQ(trackPaths[i], tracks[i].path);
    }
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
    const auto loadedTracks = createTracks(tracksToAddCount);

    EXPECT_EQ(0, playlist.getTrackCount());

    EXPECT_CALL(playlistIOMock, loadTracks(SizeIs(tracksToAddCount))).WillOnce(Return(loadedTracks));
    EXPECT_CALL(playlistIOMock, save);

    playlist.insertTracks(tracksToAdd);

    EXPECT_EQ(tracksToAddCount, playlist.getTrackCount());
}

TEST_F(PlaylistTests, insertTracksAtPosition)
{
    const std::vector<QUrl> tracksToLoad{ QUrl{ "Track1" }, QUrl{ "Track2" }, QUrl{ "Track3" } };
    const auto newTracksCount{ 3 };
    const auto newTracks = createTracks(newTracksCount);

    Playlist playlist{ "TestName", "TestPath", playlistIOMock };

    EXPECT_CALL(playlistIOMock, loadTracks(SizeIs(tracksToLoad.size()))).Times(2).WillRepeatedly(Return(newTracks));
    EXPECT_CALL(playlistIOMock, save).Times(2).WillRepeatedly(Return(true));

    playlist.insertTracks(0, tracksToLoad);
    EXPECT_EQ(newTracksCount, playlist.getTrackCount());

    playlist.insertTracks(1, tracksToLoad);

    validateTracks(playlist, { "NewTrack0", "NewTrack0", "NewTrack1", "NewTrack2", "NewTrack1", "NewTrack2" });
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

    validateTracks(playlist, { "NewTrack1", "NewTrack3" });
}

TEST_F(PlaylistTests, moveTracks)
{
    const std::vector<QUrl> tracksToLoad{};
    const auto newTracksCount{ 5 };
    const auto newTracks = createTracks(newTracksCount);

    EXPECT_CALL(playlistIOMock, loadTracks).WillOnce(Return(newTracks));
    EXPECT_CALL(playlistIOMock, save).Times(2).WillRepeatedly(Return(true));

    Playlist playlist{ "TestName", "TestPath", tracksToLoad, playlistIOMock };
    ASSERT_EQ(newTracksCount, playlist.getTrackCount());

    const std::vector<std::size_t> indexesToMove{ 0, 2, 4 };
    constexpr std::size_t moveToPosition{ 1 };
    playlist.moveTracks(indexesToMove, moveToPosition);

    validateTracks(playlist, { "NewTrack1", "NewTrack0", "NewTrack2", "NewTrack4", "NewTrack3" });
}

TEST_F(PlaylistTests, moveTracksOnNonexistentIndex)
{
    const std::vector<QUrl> tracksToLoad{};
    const auto newTracksCount{ 5 };
    const auto newTracks = createTracks(newTracksCount);

    EXPECT_CALL(playlistIOMock, loadTracks).WillOnce(Return(newTracks));
    EXPECT_CALL(playlistIOMock, save).Times(2).WillRepeatedly(Return(true));

    Playlist playlist{ "TestName", "TestPath", tracksToLoad, playlistIOMock };
    ASSERT_EQ(newTracksCount, playlist.getTrackCount());

    const std::vector<std::size_t> indexesToMove{ 0, 2, 4 };
    constexpr std::size_t moveToPosition{ 999 };
    playlist.moveTracks(indexesToMove, moveToPosition);

    validateTracks(playlist, { "NewTrack1", "NewTrack3", "NewTrack0", "NewTrack2", "NewTrack4" });
}

TEST_F(PlaylistTests, getNextTrackIndexOnEmptyPlaylist)
{
    const Playlist playlist{ "TestName", "TestPath", playlistIOMock };
    EXPECT_EQ(std::nullopt, playlist.getNextTrackIndex());
}

TEST_F(PlaylistTests, getNextTrackIndexRollsOverOnLastTrack)
{
    const std::vector<QUrl> tracksToLoad{};
    const auto newTracksCount{ 2 };
    const auto newTracks = createTracks(newTracksCount);

    EXPECT_CALL(playlistIOMock, loadTracks).WillOnce(Return(newTracks));
    EXPECT_CALL(playlistIOMock, save).WillRepeatedly(Return(true));

    Playlist playlist{ "TestName", "TestPath", tracksToLoad, playlistIOMock };

    auto nextIndex = playlist.getNextTrackIndex();
    EXPECT_EQ(0, nextIndex);
    playlist.setCurrentTrackIndex(*nextIndex);

    nextIndex = playlist.getNextTrackIndex();
    EXPECT_EQ(1, nextIndex);
    playlist.setCurrentTrackIndex(*nextIndex);

    nextIndex = playlist.getNextTrackIndex();
    EXPECT_EQ(0, nextIndex);
    playlist.setCurrentTrackIndex(*nextIndex);
}

TEST_F(PlaylistTests, getPreviousTrackIndexOnEmptyPlaylist)
{
    const Playlist playlist{ "TestName", "TestPath", playlistIOMock };
    EXPECT_EQ(std::nullopt, playlist.getPreviousTrackIndex());
}

TEST_F(PlaylistTests, getPreviousTrackIndexRollsOverOnFirstTrack)
{
    const std::vector<QUrl> tracksToLoad{};
    const auto newTracksCount{ 2 };
    const auto newTracks = createTracks(newTracksCount);

    EXPECT_CALL(playlistIOMock, loadTracks).WillOnce(Return(newTracks));
    EXPECT_CALL(playlistIOMock, save).WillRepeatedly(Return(true));

    Playlist playlist{ "TestName", "TestPath", tracksToLoad, playlistIOMock };

    auto prevIndex = playlist.getPreviousTrackIndex();
    EXPECT_EQ(1, prevIndex);
    playlist.setCurrentTrackIndex(*prevIndex);

    prevIndex = playlist.getPreviousTrackIndex();
    EXPECT_EQ(0, prevIndex);
    playlist.setCurrentTrackIndex(*prevIndex);

    prevIndex = playlist.getPreviousTrackIndex();
    EXPECT_EQ(1, prevIndex);
    playlist.setCurrentTrackIndex(*prevIndex);
}

using IndexChangeInsertParams = std::tuple<std::size_t, std::size_t, std::size_t>;

struct PlaylistInsertCurrentIndexChangeTest : public TestWithParam<IndexChangeInsertParams>
{
    StrictMock<PlaylistIOMock> playlistIOMock{};
};

INSTANTIATE_TEST_SUITE_P(CurrentTrackChangeInsert,
                         PlaylistInsertCurrentIndexChangeTest,
                         Values(IndexChangeInsertParams{ 0, 0, 2 },
                                IndexChangeInsertParams{ 0, 1, 0 },
                                IndexChangeInsertParams{ 1, 0, 3 }));

TEST_P(PlaylistInsertCurrentIndexChangeTest, currentTrackIndexChangesAfterInsert)
{
    const auto param = GetParam();
    const auto initialIndex{ std::get<0>(param) };
    const auto dropPosition{ std::get<1>(param) };
    const auto indexAfterInsert{ std::get<2>(param) };

    const std::vector<QUrl> tracksToLoad{};
    const auto newTracksCount{ 2 };
    const auto newTracks = createTracks(newTracksCount);

    EXPECT_CALL(playlistIOMock, loadTracks).WillRepeatedly(Return(newTracks));
    EXPECT_CALL(playlistIOMock, save).WillRepeatedly(Return(true));
    Playlist playlist{ "TestName", "TestPath", tracksToLoad, playlistIOMock };

    playlist.setCurrentTrackIndex(initialIndex);
    playlist.insertTracks(dropPosition, tracksToLoad);
    EXPECT_EQ(indexAfterInsert, playlist.getCurrentTrackIndex());
}

using IndexChangeRemoveParams = std::tuple<std::size_t, std::vector<std::size_t>, std::size_t>;

struct PlaylistRemoveCurrentIndexChangeTest : public TestWithParam<IndexChangeRemoveParams>
{
    StrictMock<PlaylistIOMock> playlistIOMock{};
};

INSTANTIATE_TEST_SUITE_P(CurrentTrackChangeRemove,
                         PlaylistRemoveCurrentIndexChangeTest,
                         Values(IndexChangeRemoveParams{ 1, { 1 }, 1 }, // Needs to be fixed
                                IndexChangeRemoveParams{ 1, { 2, 3 }, 1 },
                                IndexChangeRemoveParams{ 2, { 0, 1 }, 0 },
                                IndexChangeRemoveParams{ 2, { 1, 3 }, 1 }));

TEST_P(PlaylistRemoveCurrentIndexChangeTest, currentTrackIndexChangesAfterInsert)
{
    const auto param = GetParam();
    const auto initialIndex{ std::get<0>(param) };
    const auto indexesToRemove{ std::get<1>(param) };
    const auto indexAfterRemove{ std::get<2>(param) };

    const std::vector<QUrl> tracksToLoad{};
    const auto newTracksCount{ 4 };
    const auto newTracks = createTracks(newTracksCount);

    EXPECT_CALL(playlistIOMock, loadTracks).WillRepeatedly(Return(newTracks));
    EXPECT_CALL(playlistIOMock, save).WillRepeatedly(Return(true));
    Playlist playlist{ "TestName", "TestPath", tracksToLoad, playlistIOMock };

    playlist.setCurrentTrackIndex(initialIndex);
    playlist.removeTracks(indexesToRemove);
    EXPECT_EQ(indexAfterRemove, playlist.getCurrentTrackIndex());
}
