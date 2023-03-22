#ifndef RANDOM_MOVE_PLAYER_HPP
#define RANDOM_MOVE_PLAYER_HPP

#include "Player.hpp"

namespace ChessEngineLib {

class RandomMovePlayer : public Player {
public:
    std::optional<Move> getMove(Board const& board) override;
};

}

#endif
