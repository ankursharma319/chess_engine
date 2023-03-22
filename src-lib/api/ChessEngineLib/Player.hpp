#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <string>
#include <array>
#include <optional>

#include "Board.hpp"
#include "Move.hpp"

namespace ChessEngineLib {

class Player {
public:
    virtual ~Player() = default;
    virtual std::optional<Move> getMove(Board const& board) = 0;
};

}

#endif
