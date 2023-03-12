#include <nlohmann/json.hpp>
#include "Engine.hpp"

namespace ChessEngineLib {

bool validateFen(std::string const& fen) {
    std::optional<Board> board = Board::fromFen(fen);
    return board.has_value();
}

bool isMoveLegal(std::string const& fen, std::string const& move) {
    (void) fen;
    (void) move;
    return false;
}

std::size_t getNumberOfLegalMoves(std::string const& fen) {
    (void) fen;
    return 0;
}

}
