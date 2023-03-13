#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <string>
#include <unordered_set>
#include "Board.hpp"
#include "Move.hpp"

namespace ChessEngineLib {

bool validateFen(std::string const& fen);
std::unordered_set<Square> generateLegalDestinations(Board const& board, Square source);

}

#endif
