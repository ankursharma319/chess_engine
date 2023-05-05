#! /bin/sh

# https://web.stanford.edu/class/archive/cs/cs107/cs107.1174/guide_callgrind.html

cmake --build _build/debug/ \
&& valgrind \
--tool=callgrind \
--simulate-cache=yes \
./_build/debug/tests/ChessEngineTests --gtest_filter=AiPlayerTestFixture.random_move_player_can_play_long_games

# gprof2dot -f callgrind callgrind.out.158836 | dot -Tsvg -o callgrind_output.svg

# Interpreting cache output
# I1 - first level instruction cache
# D1 - first level data cache
# L2 - second level unified cache
# LL - last level cache
