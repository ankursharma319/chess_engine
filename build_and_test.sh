#! /bin/sh

cmake --build _build/
_build/src-exe/ChessEngine
cmake --install _build/ --prefix _install/
_install/bin/ChessEngine
