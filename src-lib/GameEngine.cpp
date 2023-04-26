#include "GameEngine.hpp"
#include "Board.hpp"
#include "Move.hpp"
#include "glog/logging.h"

#include <algorithm>
#include <cassert>
#include <limits>
#include <stdexcept>
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
    ChessEngineLib::Piece piece = board.at(source).value();
    assert(piece.type == ChessEngineLib::Piece::Type::Pawn);
    assert(source.row != 0 && source.row != 7);

    bool spawn_position = piece.color == ChessEngineLib::Color::White ? source.row == 1 : source.row == 6;
    std::int8_t movement_dir = piece.color == ChessEngineLib::Color::White ? 1 : -1;

    std::unordered_set<ChessEngineLib::Square> dests {};
    if (!board.grid().at(source.col).at(source.row + movement_dir).has_value()) {
        dests.insert({source.col, static_cast<std::uint8_t>(source.row + movement_dir)});
        if (spawn_position && !board.grid().at(source.col).at(source.row + movement_dir*2)) {
            dests.insert({source.col, static_cast<std::uint8_t>(source.row + movement_dir * 2)});
        }
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
            static_cast<std::uint8_t>(source.col + direction.first),
            static_cast<std::uint8_t>(source.row + direction.second)
        };
        if (dst.col >= 8 || dst.row >= 8) {
            continue;
        }
        if (board.at(dst).has_value() && board.at(dst).value().color == piece.color) {
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
bool castling_allowed(
    ChessEngineLib::Board const& board,
    ChessEngineLib::Color color,
    ChessEngineLib::Side side
) {
    if (!board.isCastlingAvailable(color, side)) {
        return false;
    }
    std::unordered_set<ChessEngineLib::Square> vacant_squares_needed {};
    if (color == ChessEngineLib::Color::Black && side == ChessEngineLib::Side::KingSide) {
        vacant_squares_needed = {{5,7},{6,7}};
    } else if (color == ChessEngineLib::Color::White && side == ChessEngineLib::Side::KingSide) {
        vacant_squares_needed = {{5,0},{6,0}};
    } else if (color == ChessEngineLib::Color::Black && side == ChessEngineLib::Side::QueenSide) {
        vacant_squares_needed = {{1,7},{2,7},{3,7}};
    } else if (color == ChessEngineLib::Color::White && side == ChessEngineLib::Side::QueenSide) {
        vacant_squares_needed = {{1,0},{2,0},{3,0}};
    } else {
        assert(false);
    }
    for (ChessEngineLib::Square square: vacant_squares_needed) {
        if (board.at(square).has_value()) {
            return false;
        }
    }
    return true;
}

std::unordered_set<ChessEngineLib::Square> king_destinations(
    ChessEngineLib::Board const& board, ChessEngineLib::Square source
) {
    using namespace ChessEngineLib;
    std::unordered_set<Square> dests =
        short_range_piece_destinations(board, source, {{-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1}});
    Piece const& piece = board.at(source).value();
    bool spawn_position = piece.color == Color::White ? source == Square({4,0}) : source == Square({4,7});
    if(castling_allowed(board, piece.color, ChessEngineLib::Side::KingSide)) {
        assert(spawn_position);
        dests.insert(Square({6,source.row}));
    }
    if(castling_allowed(board, piece.color, ChessEngineLib::Side::QueenSide)) {
        assert(spawn_position);
        dests.insert(Square({2,source.row}));
    }
    return dests;
}

bool is_pawn_move_pseudo_legal(ChessEngineLib::Board const& board, ChessEngineLib::Move const& move) {
    assert(move.fromSquare.row != 0 && move.fromSquare.row != 7);

    ChessEngineLib::Piece piece = board.at(move.fromSquare).value();
    bool spawn_position = piece.color == ChessEngineLib::Color::White ? move.fromSquare.row == 1 : move.fromSquare.row == 6;
    std::int8_t movement_dir = piece.color == ChessEngineLib::Color::White ? 1 : -1;
    std::uint8_t expected_single_move_row = move.fromSquare.row + movement_dir;

    // straight move
    if (move.fromSquare.col == move.toSquare.col) {
        if(board.at(move.toSquare).has_value()) {
            VLOG(3) << "illegal straight pawn move because there is a piece in at destination" << move.toSquare;
            return false;
        }
        std::uint8_t expected_double_move_row = expected_single_move_row + movement_dir;
        if (move.toSquare.row == expected_single_move_row) {
            if (move.toSquare.row == 7 || move.toSquare.row == 0) {
                return move.promotionTo.has_value() &&
                    move.promotionTo.value() != ChessEngineLib::Piece::Type::King &&
                    move.promotionTo.value() != ChessEngineLib::Piece::Type::Pawn;
            }
            return true;
        } else if (move.toSquare.row == expected_double_move_row) {
            return spawn_position && !board.grid().at(move.fromSquare.col).at(expected_single_move_row).has_value();
        } else {
            VLOG(3) << "illegal pawn move because pawn dont move like that: " << move;
            return false;
        }
    }
    bool is_capture = board.at(move.toSquare).has_value() ||
        (board.getEnPassantSquare().has_value() && board.getEnPassantSquare().value() == move.toSquare);
    bool is_correct_row = move.toSquare.row == expected_single_move_row;
    bool is_correct_col = std::abs(move.toSquare.col - move.fromSquare.col) == 1;
    bool is_valid_diagonal_move = is_capture && is_correct_row && is_correct_col;
    return is_valid_diagonal_move;
}

bool is_rook_move_pseudo_legal(ChessEngineLib::Board const& board, ChessEngineLib::Move const& move) {
    std::int8_t direction_col = move.toSquare.col - move.fromSquare.col;
    std::int8_t direction_row = move.toSquare.row - move.fromSquare.row;
    direction_col = std::clamp<std::int8_t>(direction_col, -1, 1);
    direction_row = std::clamp<std::int8_t>(direction_row, -1, 1);
    if ((direction_row != 0) && (direction_col != 0)) {
        VLOG(3) << "illegal move because rook dont move in the direction (" << +direction_col << ", " << +direction_row << ")";
        return false;
    }
    VLOG(5) << "moving in direction (" << +direction_col << ", " << +direction_row << ")";
    VLOG(5) << "move = " << move;
    for (
        std::uint8_t col = move.fromSquare.col + direction_col, row = move.fromSquare.row + direction_row;
        (col != move.toSquare.col) || (row != move.toSquare.row);
        col += direction_col, row += direction_row
    ) {
        VLOG(5) << "checking contents at (" << +col << ", " << +row << ")";
        assert(col < 8);
        assert(row < 8);
        if (board.grid().at(col).at(row).has_value()) {
            VLOG(3) << "illegal move because there is a piece in the way at (" << col << ", " << row << ")";
            return false;
        }
    }
    return true;
}

bool is_queen_move_pseudo_legal(ChessEngineLib::Board const& board, ChessEngineLib::Move const& move) {
    std::int8_t direction_col = move.toSquare.col - move.fromSquare.col;
    std::int8_t direction_row = move.toSquare.row - move.fromSquare.row;
    if ((std::abs(direction_col) != std::abs(direction_row)) &&
        (direction_row != 0) && (direction_col != 0)
    ) {
        VLOG(3) << "illegal move because queen dont move in the direction (" << +direction_col << ", " << +direction_row << ")";
        return false;
    }
    direction_col = std::clamp<std::int8_t>(direction_col, -1, 1);
    direction_row = std::clamp<std::int8_t>(direction_row, -1, 1);
    for (
        std::uint8_t col = move.fromSquare.col + direction_col, row = move.fromSquare.row + direction_row;
        (col != move.toSquare.col) || (row != move.toSquare.row);
        col += direction_col, row += direction_row
    ) {
        VLOG(5) << "checking contents at (" << +col << ", " << +row << ")";
        assert(col < 8);
        assert(row < 8);
        if (board.grid().at(col).at(row).has_value()) {
            VLOG(3) << "illegal move because there is a piece in the way at (" << col << ", " << row << ")";
            return false;
        }
    }
    return true;
}

bool is_bishop_move_pseudo_legal(ChessEngineLib::Board const& board, ChessEngineLib::Move const& move) {
    std::int8_t direction_col = move.toSquare.col - move.fromSquare.col;
    std::int8_t direction_row = move.toSquare.row - move.fromSquare.row;
    if (std::abs(move.toSquare.col - move.fromSquare.col) != std::abs(move.toSquare.row - move.fromSquare.row)) {
        VLOG(3) << "illegal move because bishop dont move in the direction (" << +direction_col << ", " << +direction_row << ")";
        return false;
    }
    direction_col = std::clamp<std::int8_t>(direction_col, -1, 1);
    direction_row = std::clamp<std::int8_t>(direction_row, -1, 1);
    for (
        std::uint8_t col = move.fromSquare.col + direction_col, row = move.fromSquare.row + direction_row;
        (col != move.toSquare.col) || (row != move.toSquare.row);
        col += direction_col, row += direction_row
    ) {
        assert(col < 8);
        assert(row < 8);
        VLOG(5) << "checking contents at (" << +col << ", " << +row << ")";
        if (board.grid().at(col).at(row).has_value()) {
            VLOG(3) << "illegal move because there is a piece in the way at (" << col << ", " << row << ")";
            return false;
        }
    }
    return true;
}

bool is_knight_move_pseudo_legal(ChessEngineLib::Board const&, ChessEngineLib::Move const& move) {
    std::pair<std::int8_t, std::int8_t> direction = {
        move.toSquare.col - move.fromSquare.col,
        move.toSquare.row - move.fromSquare.row
    };
    if (std::abs(direction.first) == 2 && std::abs(direction.second) == 1) {
        return true;
    }
    if (std::abs(direction.first) == 1 && std::abs(direction.second) == 2) {
        return true;
    }
    VLOG(3) << "illegal move because knight dont move in the direction (" << +direction.first << ", " << +direction.second << ")";
    return false;
}

bool is_king_move_pseudo_legal(ChessEngineLib::Board const& board, ChessEngineLib::Move const& move) {
    std::pair<std::int8_t, std::int8_t> direction = {
        move.toSquare.col - move.fromSquare.col,
        move.toSquare.row - move.fromSquare.row
    };
    // normal move
    if (std::abs(direction.first) <= 1 && std::abs(direction.second) <= 1) {
        return true;
    }

    // castle
    VLOG(3) << "not a normal king move, check if its castling and legal";
    ChessEngineLib::Piece piece = board.at(move.fromSquare).value();
    bool spawn_position = piece.color == ChessEngineLib::White ?
        move.fromSquare == ChessEngineLib::Square({4,0}) :
        move.fromSquare == ChessEngineLib::Square({4,7});
    bool row_changed = move.fromSquare.row != move.toSquare.row;
    bool castle_kingside_played = move.toSquare.col == 6 && spawn_position && !row_changed;
    bool castle_queenside_played = move.toSquare.col == 2 && spawn_position && !row_changed;
    if (castle_kingside_played) {
        return castling_allowed(board, piece.color, ChessEngineLib::Side::KingSide);
    }
    if (castle_queenside_played) {
        return castling_allowed(board, piece.color, ChessEngineLib::Side::QueenSide);
    }
    VLOG(3) << "castling also not played, not legal";
    return false;
}

}

namespace ChessEngineLib {

bool validateFen(std::string const& fen) {
    std::optional<Board> board = Board::fromFen(fen);
    return board.has_value();
}

std::unordered_set<Square> generateLegalDestinations(Board const& board, Square source) {
    VLOG(2) << "generating pseudo-legal moves for source " << source;
    std::unordered_set<Square> dsts = generatePseudoLegalDestinations(board, source);
    VLOG(2) << "found " << dsts.size() << " pseudo-legal moves from source square " << source;
    for (auto it = dsts.begin(); it != dsts.end();) {
        Square dst = *it;
        Board board_copy = board;
        Move move = Move(source, dst);
        board_copy.forceMakeMove(move);
        VLOG(4) << "check if king capture possible after theoretical move " << move;
        if (isKingCapturePossibleNextMove(board_copy)) {
            VLOG(4) << "determined that king capture possible for move " << move;
            it = dsts.erase(it);
        } else {
            VLOG(4) << "determined that move " << move << " is legal";
            it++;
        }
    }
    VLOG(2) << "found " << dsts.size() << " fully legal moves from source " << source;
    return dsts;
}

std::unordered_set<Square> generatePseudoLegalDestinations(Board const& board, Square source) {
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
    return {};
}

bool isMovePseudoLegal(Board const& board, Move const& move) {
    std::optional<Piece> const& piece = board.at(move.fromSquare);
    if (!piece.has_value()) {
        VLOG(3) << "illegal move because no piece on source";
        return false;
    }
    if (piece.value().color != board.getNextMoveColor()) {
        VLOG(3) << "illegal move because next move is not of this pieces color";
        return false;
    }
    assert(move.fromSquare.row < 8);
    assert(move.fromSquare.col < 8);
    assert(move.toSquare.row < 8);
    assert(move.toSquare.col < 8);

    if (board.at(move.toSquare).has_value() && board.at(move.toSquare).value().color == piece.value().color) {
        VLOG(3) << "illegal move because target square occupied";
        return false;
    }
    if (move.fromSquare == move.toSquare) {
        VLOG(3) << "illegal move because source square and target square are the same";
        return false;
    }
    switch (piece.value().type) {
        case Piece::Type::Pawn:
            return is_pawn_move_pseudo_legal(board, move);
        case Piece::Type::Rook:
            return is_rook_move_pseudo_legal(board, move);
        case Piece::Type::Queen:
            return is_queen_move_pseudo_legal(board, move);
        case Piece::Type::Bishop:
            return is_bishop_move_pseudo_legal(board, move);
        case Piece::Type::Knight:
            return is_knight_move_pseudo_legal(board, move);
        case Piece::Type::King:
            return is_king_move_pseudo_legal(board, move);
    }
    throw std::runtime_error("Impossible");
}

bool makeMove(Board& board, Move const& move) {
    VLOG(1) << "asked to make move " << move;
    if(!isMovePseudoLegal(board, move)) {
        return false;
    }
    Board board_before_move = board;
    board.forceMakeMove(move);
    if (isKingCapturePossibleNextMove(board)) {
        VLOG(2) << "illegal move because king would die immediately";
        board = board_before_move;
        return false;
    }
    return true;
}

bool isKingCapturePossibleNextMove(ChessEngineLib::Board const& board) {
    VLOG(4) << "trying to check if king capture is possible next move by generating pseudolegal destinations in theoretical position";
    for (std::uint8_t col=0; col<8; col++) {
        for (std::uint8_t row=0; row<8; row++) {
            std::unordered_set<Square> res = generatePseudoLegalDestinations(board, Square({col, row}));
            if (!res.empty()) {
                VLOG(7) << "found " << res.size() << " pseudo legal-moves from square at (" << +col << ", " << +row << ")";
            }
            for (Square const& sqr: res) {
                if (board.at(sqr).has_value() && board.at(sqr).value().type == Piece::Type::King) {
                    VLOG(4) << "determined that king capture is possible in theoretical position";
                    return true;
                }
            }
        }
    }
    VLOG(4) << "determined that king capture is not possible in theoretical position";
    return false;
}

std::ostream & operator<<(std::ostream &os, ResultType rt) {
    switch (rt) {
        case ResultType::Draw:
            os << "Draw";
            break;
        case ResultType::WhiteWin:
            os << "WhiteWin";
            break;
        case ResultType::BlackWin:
            os << "BlackWin";
            break;
    }
    return os;
}

std::optional<ResultType> isGameOver(Board const& board) {
    if (board.getHalfMoveClock() >= 50) {
        VLOG(2) << "game over because of 50 move rule";
        return ResultType::Draw;
    }
    VLOG(2) << "try to check if there are any legal moves to determine if match is over";
    for (std::uint8_t col=0; col<8; col++) {
        for (std::uint8_t row=0; row<8; row++) {
            std::unordered_set<Square> legal_moves = generateLegalDestinations(board, {col, row});
            if (!legal_moves.empty()) {
                VLOG(4) << "found " << legal_moves.size() << " legal moves from ("
                    << +col << ", " << +row << "), so not game over";
                return std::nullopt;
            }
        }
    }
    VLOG(2) << "no legal moves try to determine if stalemate or checkmate";
    Board board_copy = board;
    Color color = board.getNextMoveColor() == Color::White ? Color::Black : Color::White;
    board_copy.setNextMoveColor(color);
    if (!isKingCapturePossibleNextMove(board_copy)) {
        return ResultType::Draw;
    }
    if (board.getNextMoveColor() == Color::White) {
        return ResultType::BlackWin;
    } else {
        return ResultType::WhiteWin;
    }
}

std::unordered_set<Move> getAllLegalMoves(Board const& board) {
    auto is_pawn_promotion = [] (Piece const& piece, Square dst) -> bool {
        return (piece.type == Piece::Type::Pawn) &&
            ((piece.color == Color::White && dst.row == 7) ||
            (piece.color == Color::Black && dst.row == 0));
    };
    std::unordered_set<Move> legal_moves {};
    for (std::uint8_t col=0; col<8; col++) {
        for (std::uint8_t row=0; row<8; row++) {
            std::unordered_set<Square> legal_dsts = generateLegalDestinations(board, {col, row});
            for (Square dst: legal_dsts) {
                Piece const& piece = board.at({col, row}).value();
                if (is_pawn_promotion(piece, dst)) {
                    legal_moves.insert(Move(Square {col, row}, dst, Piece::Type::Rook));
                    legal_moves.insert(Move(Square {col, row}, dst, Piece::Type::Queen));
                    legal_moves.insert(Move(Square {col, row}, dst, Piece::Type::Bishop));
                    legal_moves.insert(Move(Square {col, row}, dst, Piece::Type::Knight));
                } else {
                    legal_moves.insert(Move(Square {col, row}, dst));
                }
            }
        }
    }
    return legal_moves;
}

bool isMoveLegal(Board const& board, Move const& move) {
    VLOG(1) << "asked to make move " << move;
    if(!isMovePseudoLegal(board, move)) {
        return false;
    }
    Board board_copy = board;
    board_copy.forceMakeMove(move);
    if (isKingCapturePossibleNextMove(board_copy)) {
        VLOG(2) << "illegal move because king would die immediately";
        return false;
    }
    return true;
}

}
