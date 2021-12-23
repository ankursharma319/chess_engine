#! /bin/sh

# run ./configure.sh first

cmake --build _build/debug/ \
&& gdb \
-ex 'break Board::fromFen' \
-ex 'info b' \
-ex 'set print pretty on' \
-ex 'run' \
./_build/debug/tests/ChessEngineTests

#cmake --install _build/ --prefix _install/
#_install/bin/ChessEngine
