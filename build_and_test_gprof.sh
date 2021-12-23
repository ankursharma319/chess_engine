#! /bin/sh

cmake --build _build/gprof_ready/ \
&& ./_build/gprof_ready/tests/ChessEngineTests
gprof --graph --flat-profile ./_build/gprof_ready/tests/ChessEngineTests gmon.out > gprof.out
gprof ./_build/gprof_ready/tests/ChessEngineTests | gprof2dot |  dot -Tsvg -o gprof_output.svg
