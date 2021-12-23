#! /bin/sh

# https://web.stanford.edu/class/archive/cs/cs107/cs107.1174/guide_callgrind.html

cmake --build _build/release/ \
&& valgrind \
--tool=callgrind \
--simulate-cache=yes \
./_build/release/tests/ChessEngineTests

# ./gprof2dot.py -f callgrind callgrind.out.158836 | dot -Tsvg -o output.svg
