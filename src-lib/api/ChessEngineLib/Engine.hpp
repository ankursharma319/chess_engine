#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <string>
#include "Board.hpp"
#include "Move.hpp"

namespace ChessEngineLib {

bool validateFen(std::string fen);
bool isMoveLegal(std::string fen, std::string move);
std::size_t getNumberOfLegalMoves(std::string fen);

}

#endif
