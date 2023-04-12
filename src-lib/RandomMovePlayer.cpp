#include "RandomMovePlayer.hpp"
#include "GameEngine.hpp"
#include "glog/logging.h"

#include  <random>

namespace ChessEngineLib {

std::optional<Move> RandomMovePlayer::getMove(Board const& board) {
    VLOG(2) << "getMove called on board " << board;
    std::unordered_set<Move> moves = getAllLegalMoves(board);
    if (moves.empty()) {
        return std::nullopt;
    }
    std::vector<Move> out;
    std::size_t const nelems = 1;
    std::sample(
        moves.begin(),
        moves.end(),
        std::back_inserter(out),
        nelems,
        std::mt19937{std::random_device{}()}
    );
    return std::move(out.front());
}

}

