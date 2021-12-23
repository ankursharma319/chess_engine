#! /bin/sh

cmake --build _build/debug/ \
&& valgrind \
--leak-check=full \
--show-leak-kinds=all \
--track-origins=yes \
--log-file=valgrind-out.txt \
./_build/debug/tests/ChessEngineTests
