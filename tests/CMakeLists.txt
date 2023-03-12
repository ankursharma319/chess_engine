enable_testing()

add_executable(ChessEngineTests ChessEngineTests.cpp)

target_link_libraries(ChessEngineTests gtest glog::glog ChessEngineLib)

include(GoogleTest)

gtest_discover_tests(ChessEngineTests)
