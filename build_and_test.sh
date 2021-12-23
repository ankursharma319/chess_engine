#! /bin/sh

# run ./configure.sh first

cmake --build _build/ \
&& ./_build/tests/ChessEngineTests

#./_build/src-exe/ChessEngine
#ctest --build-and-test ./ _build/ --verbose --build-generator "Unix Makefiles"
#cmake --install _build/ --prefix _install/
#_install/bin/ChessEngine
