{ pkgs ? import (fetchTarball "https://github.com/NixOS/nixpkgs/archive/7ac6901f5e3038d59d3e2576af445fe418291dd8.tar.gz") {}}:

pkgs.mkShell {
    nativeBuildInputs = [
        pkgs.cmake
    ];
    buildInputs = [
        pkgs.gtest
        pkgs.nlohmann_json
        pkgs.gdb
        pkgs.valgrind
    ];
}
