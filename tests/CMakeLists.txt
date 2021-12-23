enable_testing()

add_executable(ChessEngineTests ChessBoardTests.cpp)

target_link_libraries(ChessEngineTests gtest_main)
target_link_libraries(ChessEngineTests ChessEngineLib)

include(GoogleTest)

gtest_discover_tests(ChessEngineTests)
