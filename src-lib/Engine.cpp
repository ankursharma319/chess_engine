#include "Engine.hpp"
#include "Board.hpp"
#include "Move.hpp"
#include "glog/logging.h"
#include <cassert>
#include <limits>
#include <unordered_set>
#include <utility>
#include <valarray>
#include <vector>

namespace {

std::unordered_set<ChessEngineLib::Square> long_range_piece_destinations(
    ChessEngineLib::Board const& board, ChessEngineLib::Square source,
    std::vector<std::pair<std::uint8_t, std::uint8_t>> const& directions
) {
    ChessEngineLib::Color piece_color = board.at(source).value().color;
    std::unordered_set<ChessEngineLib::Square> dests = {};
    for (auto const& direction: directions) {
        for (
            std::uint8_t col=source.col+direction.first, row=source.row+direction.second;
            (col <= 7) && (row <= 7);
            col += direction.first, row += direction.second
        ) {
            if (!board.grid().at(col).at(row).has_value()) {
                dests.insert({col, row});
            } else if (board.grid().at(col).at(row).value().color == piece_color) {
                break;
            } else {
                dests.insert({col, row});
                break;
            }
        }
    }
    return dests;
}

std::unordered_set<ChessEngineLib::Square> queen_destinations(
    ChessEngineLib::Board const& board, ChessEngineLib::Square source
) {
    // for each of the 8 directions, move til the end or til encounter a piece
    return long_range_piece_destinations(board, source, {{-1,-1}, {-1,0}, {-1,1}, {0,-1}, {0,1}, {1,-1}, {1,0}, {1,1}});
}

std::unordered_set<ChessEngineLib::Square> rook_destinations(
    ChessEngineLib::Board const& board, ChessEngineLib::Square source
) {
    // for each of the 4 directions, move til the end or til encounter a piece
    return long_range_piece_destinations(board, source, {{-1,0}, {0,-1}, {0,1}, {1,0}});
}

std::unordered_set<ChessEngineLib::Square> bishop_destinations(
    ChessEngineLib::Board const& board, ChessEngineLib::Square source
) {
    // for each of the 4 directions, move til the end or til encounter a piece
    return long_range_piece_destinations(board, source, {{-1,-1}, {-1,1}, {1,-1}, {1,1}});
}

std::unordered_set<ChessEngineLib::Square> pawn_destinations(
    ChessEngineLib::Board const& board, ChessEngineLib::Square source
) {
    // TODO: count different promotions as different moves
    ChessEngineLib::Piece piece = board.at(source).value();
    assert(piece.type == ChessEngineLib::Piece::Type::Pawn);
    assert(source.row != 0 && source.row != 7);

    bool spawn_position = piece.color == ChessEngineLib::Color::White ? source.row == 1 : source.row == 6;
    std::int8_t movement_dir = piece.color == ChessEngineLib::Color::White ? 1 : -1;

    std::unordered_set<ChessEngineLib::Square> dests {};
    if (!board.grid().at(source.col).at(source.row + movement_dir).has_value()) {
        dests.insert({source.col, static_cast<std::uint8_t>(source.row + movement_dir)});
    }
    if (spawn_position && !board.grid().at(source.col).at(source.row + movement_dir*2)) {
        dests.insert({source.col, static_cast<std::uint8_t>(source.row + movement_dir * 2)});
    }
    if (board.getEnPassantSquare().has_value() &&
        board.getEnPassantSquare().value().row == source.row + movement_dir &&
        std::abs(board.getEnPassantSquare().value().col - source.col) <= 1
    ) {
        dests.insert(board.getEnPassantSquare().value());
    }
    // try capture left
    if ((source.col > 0)) {
        std::optional<ChessEngineLib::Piece> diagonal_piece = board.grid().at(source.col-1).at(source.row + movement_dir);
        if (diagonal_piece.has_value() && diagonal_piece.value().color != piece.color) {
            dests.insert(ChessEngineLib::Square{
                static_cast<uint8_t>(source.col - 1),
                static_cast<uint8_t>(source.row + movement_dir)
            });
        }
    }
    // try capture right
    if ((source.col < 7)) {
        std::optional<ChessEngineLib::Piece> diagonal_piece = board.grid().at(source.col+1).at(source.row + movement_dir);
        if (diagonal_piece.has_value() && diagonal_piece.value().color != piece.color) {
            dests.insert(ChessEngineLib::Square{
                static_cast<uint8_t>(source.col + 1),
                static_cast<uint8_t>(source.row + movement_dir)
            });
        }
    }
    return dests;
}

std::unordered_set<ChessEngineLib::Square> short_range_piece_destinations(
    ChessEngineLib::Board const& board, ChessEngineLib::Square source,
    std::vector<std::pair<std::uint8_t, std::uint8_t>> const& directions
) {
    ChessEngineLib::Piece piece = board.at(source).value();
    assert(piece.type == ChessEngineLib::Piece::Type::King ||
           piece.type == ChessEngineLib::Piece::Type::Knight);

    std::unordered_set<ChessEngineLib::Square> dests {};
    for (auto const& direction: directions) {
        ChessEngineLib::Square dst = {
            static_cast<uint8_t>(source.col + direction.first),
            static_cast<uint8_t>(source.row + direction.second)
        };
        if (dst.col >= 8 || dst.row >= 8) {
            continue;
        }
        if (board.at(dst).has_value()) {
            continue;
        }
        dests.insert(std::move(dst));
    }
    return dests;
}

std::unordered_set<ChessEngineLib::Square> knight_destinations(
    ChessEngineLib::Board const& board, ChessEngineLib::Square source
) {
    return short_range_piece_destinations(board, source, {{-2,-1},{-2,1},{-1,-2},{-1,2},{1,-2},{1,2},{2,-1},{2,1}});
}

std::unordered_set<ChessEngineLib::Square> king_destinations(
    ChessEngineLib::Board const& board, ChessEngineLib::Square source
) {
    return short_range_piece_destinations(board, source, {{-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1}});
}

}

namespace ChessEngineLib {

bool validateFen(std::string const& fen) {
    std::optional<Board> board = Board::fromFen(fen);
    return board.has_value();
}

std::unordered_set<Square> generateLegalDestinations(Board const& board, Square source) {
    std::optional<Piece> const& piece = board.grid().at(source.col).at(source.row);
    if (!piece.has_value()) {
        return {};
    }
    if (piece.value().color != board.getNextMoveColor()) {
        return {};
    }
    switch (piece.value().type) {
        case Piece::Type::Pawn:
            return pawn_destinations(board, source);
        case Piece::Type::Rook:
            return rook_destinations(board, source);
        case Piece::Type::Queen:
            return queen_destinations(board, source);
        case Piece::Type::Bishop:
            return bishop_destinations(board, source);
        case Piece::Type::Knight:
            return knight_destinations(board, source);
        case Piece::Type::King:
            return king_destinations(board, source);
    }
    // TODO For all these moves consider checks
    // TODO count castling moves
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
