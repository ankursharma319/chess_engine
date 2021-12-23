add_library(ChessEngineLib)

target_include_directories(ChessEngineLib INTERFACE api PRIVATE api/ChessEngineLib include)

target_sources(ChessEngineLib PRIVATE ChessBoard.cpp)

#install(TARGETS ChessEngineLib DESTINATION lib)
#install(FILES api/ChessEngineLib/ChessBoard.hpp DESTINATION include/ChessEngineLib)

target_link_libraries(ChessEngineLib PRIVATE nlohmann_json::nlohmann_json)
