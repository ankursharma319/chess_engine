#! /bin/sh

# run ./configure.sh first

cmake --build _build/release/ \
&& ./_build/release/tests/ChessEngineTests

#cmake --install _build/ --prefix _install/
#_install/bin/ChessEngine
