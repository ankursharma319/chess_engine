{ pkgs ? import (fetchTarball "https://github.com/NixOS/nixpkgs/archive/2ce9b9842b5e63884dfc3dea6689769e2a1ea309.tar.gz") {}}:

pkgs.mkShell {
    nativeBuildInputs = [
        pkgs.cmake
    ];
    buildInputs = if pkgs.stdenv.isDarwin then [] else [ pkgs.valgrind ] ++ [
        pkgs.gdb
        pkgs.valgrind
        pkgs.massif-visualizer
        pkgs.python39Packages.gprof2dot
        pkgs.xdot
        pkgs.graphviz
        pkgs.clang_13
    ];
}
