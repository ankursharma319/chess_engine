#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <string>
#include <unordered_set>
#include "Board.hpp"
#include "Move.hpp"

namespace ChessEngineLib {

bool validateFen(std::string const& fen);
std::unordered_set<Square> generateLegalDestinations(Board const& board, Square source);
// pseudo-legal is union of legal moves and moves which allow king capture next move
std::unordered_set<Square> generatePseudoLegalDestinations(Board const& board, Square source);

}

#endif
