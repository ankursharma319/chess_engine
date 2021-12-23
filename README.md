# Chess Engine

## Quick start

I recommend using a nix shell as the dev environment, but its not strictly necessary.

```
nix-shell shell.nix
cmake -S ./ -B ./_build/
cmake --build _build/
make -C _build/
_build/ChessEngine
```

OR

```
nix-shell shell.nix
./configure.sh
./build_and_test_release.sh
```
