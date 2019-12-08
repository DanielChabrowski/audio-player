set(TEST_FILES
    TestPlaylist.cpp
)

add_executable(unit-tests ${TEST_FILES})

target_link_libraries(unit-tests
  PRIVATE
    GTest::GTest
    GTest::Main
)

add_test(NAME UT COMMAND unit-tests)
