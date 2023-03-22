#include "RandomMovePlayer.hpp"

namespace ChessEngineLib {

std::optional<Move> RandomMovePlayer::getMove(Board const& board) {
    (void) board;
    return std::nullopt;
}

}

