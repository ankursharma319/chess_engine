#! /bin/sh

# run ./configure.sh first

cmake --build _build/debug/ \
&& cmake --build _build/release/ \
&& ./_build/debug/tests/ChessEngineTests

#./_build/src-exe/ChessEngine
#ctest --build-and-test ./ _build/ --verbose --build-generator "Unix Makefiles"
#cmake --install _build/ --prefix _install/
#_install/bin/ChessEngine
