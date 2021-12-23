#! /bin/sh

cmake --log-level=VERBOSE -S ./ -B ./_build/debug/ -D CMAKE_BUILD_TYPE=Debug
cmake --log-level=VERBOSE -S ./ -B ./_build/release/ -D CMAKE_BUILD_TYPE=Release
cmake --log-level=VERBOSE -S ./ -B ./_build/gprof_ready/ -D CMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=-pg -DCMAKE_EXE_LINKER_FLAGS=-pg -DCMAKE_SHARED_LINKER_FLAGS=-pg
