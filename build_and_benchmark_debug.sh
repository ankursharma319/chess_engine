#! /bin/sh

# run ./configure.sh first

cmake --build _build/debug/ \
&& ./_build/debug/benchmark/ChessEngineBenchmarks --benchmark_filter=.*
