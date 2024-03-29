#ifndef GAME_ENGINE_HPP
#define GAME_ENGINE_HPP

#include <string>
#include <unordered_set>
#include <ostream>

#include "Board.hpp"
#include "Move.hpp"

namespace ChessEngineLib {

bool validateFen(std::string const& fen);
std::unordered_set<Square> generateLegalDestinations(Board const& board, Square source);
// pseudo-legal is union of legal moves and moves which allow king capture next move
std::unordered_set<Square> generatePseudoLegalDestinations(Board const& board, Square source);
std::unordered_set<Move> getAllLegalMoves(Board const& board);

bool makeMove(Board& board, Move const& move);
bool isMovePseudoLegal(Board const& board, Move const& move);
bool isMoveLegal(Board const& board, Move const& move);
bool isKingCapturePossibleNextMove(Board const& board);

enum class ResultType {
    Draw, WhiteWin, BlackWin
};
std::ostream & operator<<(std::ostream &os, ResultType rt);

std::optional<ResultType> isGameOver(Board const& board);

}

#endif
