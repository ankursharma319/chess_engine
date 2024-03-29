#! /bin/sh

cmake --build _build/debug/ \
&& valgrind \
--leak-check=full \
--show-leak-kinds=all \
--track-origins=yes \
--log-file=valgrind-out.txt \
./_build/debug/tests/ChessEngineTests --gtest_filter=AiPlayerTestFixture.random_move_player_can_play_long_games

# cat valgrind-out.txt
