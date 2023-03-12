#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <string>
#include "Board.hpp"
#include "Move.hpp"

namespace ChessEngineLib {

bool validateFen(std::string const& fen);
bool isMoveLegal(std::string const& fen, std::string const& move);
std::size_t getNumberOfLegalMoves(std::string const& fen);

}

#endif
