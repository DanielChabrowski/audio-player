set(TEST_FILES
    TestPlaylist.cpp
    mocks/PlaylistIOMock.hpp
)

add_executable(core-tests ${TEST_FILES})

target_link_libraries(core-tests
  PRIVATE
    GTest::GTest
    GMock::GMock
    GMock::Main
    player::core
)

add_test(NAME UT COMMAND core-tests)
