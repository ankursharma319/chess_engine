#include "Engine.hpp"
#include "Board.hpp"
#include <unordered_set>

namespace {

std::unordered_set<ChessEngineLib::Square> rook_destinations(
    ChessEngineLib::Board const& board, ChessEngineLib::Square source
) {
    (void) source;
    (void) board;
    return {};
}

}

namespace ChessEngineLib {

bool validateFen(std::string const& fen) {
    std::optional<Board> board = Board::fromFen(fen);
    return board.has_value();
}

std::unordered_set<Square> generateLegalDestDnations(Board const& board, Square source) {
    std::optional<Piece> const& piece = board.grid().at(source.col).at(source.row);
    if (!piece.has_value()) {
        return {};
    }
    if (piece.value().color != board.getNextMoveColor()) {
        return {};
    }
    if (piece.value().type != Piece::Type::Rook) {
        return rook_destinations(board, source);
    }
    return {};
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
