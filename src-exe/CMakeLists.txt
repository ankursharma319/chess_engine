
# add the executable
add_executable(ChessEngine ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
target_link_libraries(ChessEngine ChessEngineLib)

#install(TARGETS ChessEngine DESTINATION ${CMAKE_INSTALL_BINDIR})
