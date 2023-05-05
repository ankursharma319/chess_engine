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
./build_and_benchmark_debug.sh
```

For cleanup

```bash
rm -rf _build/ && rm compile_commands.json && rm -r .cache/
rm valgrind-out.txt output.svg massif.out.* gprof.out gprof_output.svg gmon.out callgrind.out.*
```

## Notes

Memory usage for `AiPlayerTestFixture.random_move_player_can_play_long_games` according to Valgrind, pre-optimization is:

```
==143057== HEAP SUMMARY:
==143057==     in use at exit: 0 bytes in 0 blocks
==143057==   total heap usage: 469,895 allocs, 469,895 frees, 15,260,412 bytes allocated
```

max mem usage according to massif: `155.6 KB`

callgrind-cache

```
==197466== Events    : Ir Dr Dw I1mr D1mr D1mw ILmr DLmr DLmw
==197466== Collected : 256995295 65111990 43061599 309652 95521 22485 4271 9978 4101
==197466==
==197466== I   refs:      256,995,295
==197466== I1  misses:        309,652
==197466== LLi misses:          4,271
==197466== I1  miss rate:        0.12%
==197466== LLi miss rate:        0.00%
==197466==
==197466== D   refs:      108,173,589  (65,111,990 rd + 43,061,599 wr)
==197466== D1  misses:        118,006  (    95,521 rd +     22,485 wr)
==197466== LLd misses:         14,079  (     9,978 rd +      4,101 wr)
==197466== D1  miss rate:         0.1% (       0.1%   +        0.1%  )
==197466== LLd miss rate:         0.0% (       0.0%   +        0.0%  )
==197466==
==197466== LL refs:           427,658  (   405,173 rd +     22,485 wr)
==197466== LL misses:          18,350  (    14,249 rd +      4,101 wr)
==197466== LL miss rate:          0.0% (       0.0%   +        0.0%  )
```

callgrind dist:

```
ChessEngineTests
ChessEngineLib::Game::makeMove(ChessEngineLib::Move const&)
24.18%
(0.24%)
356×

ChessEngineTests
ChessEngineLib::getAllLegalMoves(ChessEngineLib::Board const&)
71.44%
(0.41%)
356×

ChessEngineTests
ChessEngineLib::generateLegalDestinations(ChessEngineLib::Board const&, ChessEngineLib::Square)
81.41%
(1.17%)
27205×

ChessEngineTests
ChessEngineLib::isKingCapturePossibleNextMove(ChessEngineLib::Board const&)
82.96%
(10.86%)
11369×

ChessEngineTests
ChessEngineLib::generatePseudoLegalDestinations(ChessEngineLib::Board const&, ChessEngineLib::Square)
54.23%
(17.38%)
721060×
```

Benchmark run:

```
BM_PlayingGameUsingRandomMovePlayer      64842 ns        64842 ns         8969
```
