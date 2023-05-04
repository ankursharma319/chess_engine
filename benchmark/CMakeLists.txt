
add_executable(ChessEngineBenchmarks BenchmarkMain.cpp)
target_link_libraries(ChessEngineBenchmarks glog::glog benchmark::benchmark ChessEngineLib)

