#include <iostream>
#include <benchmark/benchmark.h>

#include <glog/logging.h>
#include "ChessEngineLib/Game.hpp"
#include "ChessEngineLib/RandomMovePlayer.hpp"

using namespace ChessEngineLib;

static void BM_PlayingGameUsingRandomMovePlayer(benchmark::State& state) {
    RandomMovePlayer rmp = RandomMovePlayer();
    Game game {};
    for (auto _ : state) {

        if (game.result().has_value()) {
            game = Game();
        }
        // VLOG(1) << "Retrieving move#" << game.movesSize() << " for board " << game.board();
        std::optional<Move> move_opt = rmp.getMove(game.board());
        // VLOG(1) << "Retrieved move#" << game.movesSize() << " for board " << game.board();
        if(!move_opt.has_value()) {
            continue;
        }
        // VLOG(1) << "Making move#" << game.movesSize() + 1;
        game.makeMove(move_opt.value());
        // VLOG(1) << "Finished making move#" << game.movesSize() + 1;
    }
}

// Register the function as a benchmark
BENCHMARK(BM_PlayingGameUsingRandomMovePlayer);

int main(int argc, char** argv) {
    // INFO=0, WARNING=1, ERROR=2, and FATAL=3
    FLAGS_stderrthreshold = 2; // also log to stderr, if only logging to file
    FLAGS_logtostderr = false; // instead of file
    FLAGS_minloglevel = 0; // lower is more logging
    FLAGS_v = 0; // applies to VLOG, higher is more logging
    google::InitGoogleLogging(argv[0]);

    char arg0_default[] = "benchmark";
    char* args_default = arg0_default;
    if (!argv) {
      argc = 1;
      argv = &args_default;
    }
    benchmark::Initialize(&argc, argv);
    if (benchmark::ReportUnrecognizedArguments(argc, argv)) {
        return 1;
    }
    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
    return 0;
}

