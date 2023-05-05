#! /bin/sh

cmake --build _build/debug/ \
&& valgrind \
--tool=massif \
./_build/debug/tests/ChessEngineTests --gtest_filter=AiPlayerTestFixture.random_move_player_can_play_long_games

# massif-visualizer massif.out.144549

