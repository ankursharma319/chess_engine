#include "Board.hpp"
#include "GameEngine.hpp"
#include "Move.hpp"

#include <nlohmann/json.hpp>
#include <glog/logging.h>

#include <cstdint>
#include <optional>
#include <sstream>

namespace {

std::vector<std::string> split_string(
    const std::string & x,
    const std::string delim
) {
    std::vector<std::string> chunks {};
    auto start = 0U;
    auto end = x.find(delim);
    while (end != std::string::npos) {
        chunks.push_back(x.substr(start, end - start));
        start = end + delim.length();
        end = x.find(delim, start);
    }
    chunks.push_back(x.substr(start, end - start));
    return chunks;
}

std::optional<std::size_t> to_num(std::string const& s) {
    try {
        return std::stoull(s);
    } catch(std::invalid_argument const&) {
        return std::nullopt;
    } catch(std::out_of_range const& ex) {
        return std::nullopt;
    }
}

std::optional<ChessEngineLib::Color> to_color(std::string const& s) {
    if (s == "w") {
        return ChessEngineLib::Color::White;
    } else if (s == "b") {
        return ChessEngineLib::Color::Black;
    }
    return std::nullopt;
}

std::optional<ChessEngineLib::Square> get_square(std::string const& s) {
    if (s.size() != 2) {
        return std::nullopt;
    }
    if (s.front() < 'a' || s.front() > 'h') {
        return std::nullopt;
    }
    if (s.back() < '1' || s.back() > '8') {
        return std::nullopt;
    }
    std::uint8_t file = s.front() - 'a';
    std::uint8_t rank = s.back() - '1';
    assert(file < 8);
    assert(rank < 8);
    return ChessEngineLib::Square { file, rank };
}

std::optional<ChessEngineLib::Piece> parse_piece(char c) {
    using namespace ChessEngineLib;
    switch (c) {
        case 'r':
            return Piece(Piece::Type::Rook, Color::Black);
        case 'n':
            return Piece(Piece::Type::Knight, Color::Black);
        case 'b':
            return Piece(Piece::Type::Bishop, Color::Black);
        case 'q':
            return Piece(Piece::Type::Queen, Color::Black);
        case 'k':
            return Piece(Piece::Type::King, Color::Black);
        case 'p':
            return Piece(Piece::Type::Pawn, Color::Black);
        case 'R':
            return Piece(Piece::Type::Rook, Color::White);
        case 'N':
            return Piece(Piece::Type::Knight, Color::White);
        case 'B':
            return Piece(Piece::Type::Bishop, Color::White);
        case 'Q':
            return Piece(Piece::Type::Queen, Color::White);
        case 'K':
            return Piece(Piece::Type::King, Color::White);
        case 'P':
            return Piece(Piece::Type::Pawn, Color::White);
        default:
            return std::nullopt;
    }
}

std::optional<ChessEngineLib::Board::Board2dArray> parse_pieces(std::string const& fen_chunk) {
    const std::vector<std::string> ranks = split_string(fen_chunk, "/");
    if (ranks.size() != 8) {
        VLOG(3) << "didnt find 8 ranks";
        return std::nullopt;
    }
    std::array<std::array<std::optional<ChessEngineLib::Piece>, 8>, 8> grid {};
    for (std::size_t rank=0; rank < 8; rank++) {
        std::string pieces = ranks.at(rank);
        if (pieces.empty() || pieces.size() > 8) {
            VLOG(3) << "bad rank string = " << pieces;
            return std::nullopt;
        }
        std::uint8_t file = 0;
        for (char c : pieces) {
            if (file > 8) {
                VLOG(3) << "bad file = " << file;
                return std::nullopt;
            }
            if (c >= '1' && c <= '8') {
                file += (c - '0');
            } else if (auto piece = parse_piece(c); piece.has_value()) {
                assert(file < 8);
                grid[file][7-rank] = piece.value();
                file ++;
            } else {
                VLOG(3) << "bad character in rank = " << c;
                return std::nullopt;
            }
        }
    }
    return grid;
}

void write_grid_fen_chunk(std::ostream& os, ChessEngineLib::Board::Board2dArray const& grid) {
    for (std::uint8_t row=7; row <= 7; row--) {
        std::uint8_t current_gap = 0;
        for (std::uint8_t col=0; col < 8; col++) {
            if (grid[col][row].has_value()) {
                if (current_gap != 0) {
                    os << +current_gap;
                }
                os << grid[col][row].value().fen_symbol();
                current_gap = 0;
            } else {
                current_gap ++;
            }
        }
        if (current_gap != 0) {
            os << +current_gap;
        }
        if (row != 0) { os << "/"; }
    }
}

void write_next_move_color_fen_chunk(std::ostream& os, ChessEngineLib::Color color) {
    if (color == ChessEngineLib::Color::White) {
        os << "w";
    } else  {
        os << "b";
    }
}

void write_castling_availability_fen_chunk(std::ostream& os, ChessEngineLib::Board const& board) {
    bool atleast_one = false;
    if (board.isCastlingAvailable(ChessEngineLib::Color::White, ChessEngineLib::Side::KingSide)) {
        os << "K";
        atleast_one = true;
    }
    if (board.isCastlingAvailable(ChessEngineLib::Color::White, ChessEngineLib::Side::QueenSide)) {
        os << "Q";
        atleast_one = true;
    }
    if (board.isCastlingAvailable(ChessEngineLib::Color::Black, ChessEngineLib::Side::KingSide)) {
        os << "k";
        atleast_one = true;
    }
    if (board.isCastlingAvailable(ChessEngineLib::Color::Black, ChessEngineLib::Side::QueenSide)) {
        os << "q";
        atleast_one = true;
    }
    if (!atleast_one) {
        os << "-";
    }
}

void write_en_passant_fen_chunk(std::ostream& os, std::optional<ChessEngineLib::Square> const& en_passant_square) {
    if (!en_passant_square.has_value()) {
        os << "-";
        return;
    }
    ChessEngineLib::Square square = en_passant_square.value();
    os << square.pgn_file() << square.pgn_rank();
}

}

namespace ChessEngineLib {

Board Board::startingPosBoard() {
    return fromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1").value();
}

std::optional<Board> Board::fromFen(std::string const& fen) {
    VLOG(2) << "fen = " << fen;
    std::vector<std::string> chunks = split_string(fen, " ");
    if (chunks.size() != 6) {
        VLOG(2) << "fen invalid because chunks.size()=" << chunks.size();
        return std::nullopt;
    }

    // parse piece grid
    std::optional<Board2dArray> grid = parse_pieces(chunks.at(0));
    if (!grid.has_value()) {
        VLOG(2) << "fen invalid because of grid contents";
        return std::nullopt;
    }

    // parse next move color
    std::optional<Color> next_move_color = to_color(chunks.at(1));
    if (!next_move_color.has_value()) {
        VLOG(2) << "fen invalid because of next_move_color";
        return std::nullopt;
    }

    // parse castling availability
    std::optional<CastlingAvailability> castling_availability = parse_castling_availability(chunks.at(2));
    if (!castling_availability.has_value()) {
        VLOG(2) << "fen invalid because of castling_availability";
        return std::nullopt;
    }

    // parse en passant target square
    std::optional<Square> en_passant_square = std::nullopt;
    if (chunks.at(3) != "-") {
        en_passant_square = get_square(chunks.at(3));
        if (!en_passant_square.has_value()) {
            VLOG(2) << "fen invalid because of en_passant_square";
            return std::nullopt;
        }
    }

    // parse move clocks
    std::optional<std::size_t> half_move_clock = to_num(chunks.at(4));
    std::optional<std::size_t> full_move_number = to_num(chunks.at(5));
    if (!half_move_clock.has_value() || !full_move_number.has_value()) {
        VLOG(2) << "fen invalid because of move number";
        return std::nullopt;
    }

    Board board;
    board.m_grid = grid.value();
    board.m_nextMoveColor = next_move_color.value();
    board.m_castlingAvailability = castling_availability.value();
    board.m_halfMoveClock = half_move_clock.value();
    board.m_moveNumber = full_move_number.value();
    board.m_enPassantSquare = en_passant_square;
    return board;
}

std::array<std::array<std::optional<Piece>, 8>, 8> const& Board::grid() const {
    return m_grid;
}

Color Board::getNextMoveColor() const {
    return m_nextMoveColor;
}

bool Board::isCastlingAvailable(Color color, Side side) const {
    if (color == Color::Black && side == Side::KingSide) {
        return m_castlingAvailability.blackKingSide;
    } else if (color == Color::Black && side == Side::QueenSide) {
        return m_castlingAvailability.blackQueenSide;
    } else if (color == Color::White && side == Side::QueenSide) {
        return m_castlingAvailability.whiteQueenSide;
    } else {
        return m_castlingAvailability.whiteKingSide;
    }
}

std::size_t Board::getHalfMoveClock() const {
    return m_halfMoveClock;
}

std::size_t Board::getMoveNumber() const {
    return m_moveNumber;
}

std::optional<Square> Board::getEnPassantSquare() const {
    return m_enPassantSquare;
}

std::optional<Board::CastlingAvailability> Board::parse_castling_availability(std::string const& fen_chunk) {
    if (fen_chunk.empty() || (fen_chunk.size() > 4)) {
        return std::nullopt;
    }

    CastlingAvailability result {false, false, false, false};
    if (fen_chunk == "-") {
        return result;
    }
    for (std::size_t i=0; i<fen_chunk.size(); i++) {
        char c = fen_chunk.at(i);
        if (c == 'k') {
            result.blackKingSide = true;
        } else if (c == 'K') {
            result.whiteKingSide = true;
        } else if (c == 'q') {
            result.blackQueenSide = true;
        } else if (c == 'Q') {
            result.whiteQueenSide = true;
        } else {
            return std::nullopt;
        }
    }
    return result;
}

std::optional<Piece> const& Board::at(Square square) const {
    return m_grid.at(square.col).at(square.row);
}

std::string Board::fen() const {
    std::ostringstream oss;
    oss << *this;
    return oss.str();
}

std::string Board::fenWithoutMoveNumbers() const {
    std::ostringstream os;
    write_grid_fen_chunk(os, grid());
    os << " ";
    write_next_move_color_fen_chunk(os, getNextMoveColor());
    os << " ";
    write_castling_availability_fen_chunk(os, *this);
    os << " ";
    write_en_passant_fen_chunk(os, getEnPassantSquare());
    return os.str();
}

bool Board::CastlingAvailability::operator==(const Board::CastlingAvailability& other) const {
    return
        (blackKingSide == other.blackKingSide) &&
        (whiteKingSide == other.whiteKingSide) &&
        (blackQueenSide == other.blackQueenSide) &&
        (whiteQueenSide == other.whiteQueenSide);
}

bool Board::operator==(const Board& other) const {
    return
        (grid() == other.grid()) &&
        (m_castlingAvailability == other.m_castlingAvailability) &&
        (getNextMoveColor() == other.getNextMoveColor()) &&
        (getMoveNumber() == other.getMoveNumber()) &&
        (getHalfMoveClock() == other.getHalfMoveClock()) &&
        (getEnPassantSquare() == other.getEnPassantSquare());
}

bool Board::operator!=(const Board& other) const {
    return !(*this == other);
}

std::ostream & operator<<(std::ostream &os, Board const& b) {
    write_grid_fen_chunk(os, b.grid());
    os << " ";
    write_next_move_color_fen_chunk(os, b.getNextMoveColor());
    os << " ";
    write_castling_availability_fen_chunk(os, b);
    os << " ";
    write_en_passant_fen_chunk(os, b.getEnPassantSquare());
    os << " " << b.getHalfMoveClock();
    os << " " << b.getMoveNumber();
    return os;
}

void Board::expireCastlingAvailability(Color color, Side side) {
    if (color == Color::Black && side == Side::KingSide) {
        m_castlingAvailability.blackKingSide = false;
    } else if (color == Color::Black && side == Side::QueenSide) {
        m_castlingAvailability.blackQueenSide = false;
    } else if (color == Color::White && side == Side::QueenSide) {
        m_castlingAvailability.whiteQueenSide = false;
    } else {
        m_castlingAvailability.whiteKingSide = false;
    }
}

// No validation done whether the move is legal or not
void Board::forceMakeMove(Move const& move) {
    VLOG(6) << "Asked to forceMakeMove move " << move << " on board " << *this;
    Piece piece = at(move.fromSquare).value();
    m_nextMoveColor = m_nextMoveColor == Color::Black ? Color::White : Color::Black;
    bool is_capture = at(move.toSquare).has_value();
    bool is_pawn_move = piece.type == Piece::Type::Pawn;
    if (is_capture || is_pawn_move) {
        m_halfMoveClock = 0;
    } else {
        m_halfMoveClock ++;
    }
    if (is_capture) {
        assert(at(move.toSquare).value().color != piece.color);
    }
    if (piece.color == Color::Black) {
        m_moveNumber ++;
    }
    if (piece.type == Piece::Type::King) {
        expireCastlingAvailability(piece.color, Side::KingSide);
        expireCastlingAvailability(piece.color, Side::QueenSide);
    }
    if (piece.type == Piece::Type::Rook) {
        if (move.fromSquare.col == 0) {
            expireCastlingAvailability(piece.color, Side::QueenSide);
        } else if (move.fromSquare.col == 7) {
            expireCastlingAvailability(piece.color, Side::KingSide);
        }
    }
    if (is_capture && at(move.toSquare).value().type == Piece::Type::Rook) {
        if (move.toSquare.col == 0) {
            expireCastlingAvailability(at(move.toSquare).value().color, Side::QueenSide);
        } else if (move.toSquare.col == 7) {
            expireCastlingAvailability(at(move.toSquare).value().color, Side::KingSide);
        }
    }

    if (piece.type == Piece::Type::Pawn &&
        m_enPassantSquare.has_value() &&
        move.toSquare == m_enPassantSquare.value()
    ) {
        assert(!at(m_enPassantSquare.value()).has_value());
        m_grid[move.toSquare.col][move.fromSquare.row] = std::nullopt;
    }

    if (piece.type == Piece::Type::Pawn && std::abs(move.fromSquare.row - move.toSquare.row) == 2) {
        std::uint8_t pawn_direction = piece.color == Color::Black ? -1 : 1;
        m_enPassantSquare = Square({move.fromSquare.col, static_cast<uint8_t>(move.fromSquare.row + pawn_direction)});
    } else {
        m_enPassantSquare = std::nullopt;
    }

    m_grid[move.fromSquare.col][move.fromSquare.row] = std::nullopt;
    m_grid[move.toSquare.col][move.toSquare.row] = piece;

    bool is_castling = piece.type == Piece::Type::King && std::abs(move.fromSquare.col - move.toSquare.col) >= 2;
    if (is_castling) {
        assert(move.fromSquare.row == move.toSquare.row);
        assert(move.fromSquare.col == 4);
        if (move.toSquare.col == 6) {
            assert(grid().at(7).at(move.toSquare.row).has_value());
            assert(grid().at(7).at(move.toSquare.row).value().type == Piece::Type::Rook);
            assert(!grid().at(5).at(move.toSquare.row).has_value());
            m_grid[5][move.toSquare.row] = m_grid.at(7).at(move.toSquare.row);
            m_grid[7][move.toSquare.row] = std::nullopt;
        } else {
            assert(move.toSquare.col == 2);
            assert(grid().at(0).at(move.toSquare.row).has_value());
            assert(grid().at(0).at(move.toSquare.row).value().type == Piece::Type::Rook);
            assert(!grid().at(1).at(move.toSquare.row).has_value());
            assert(!grid().at(3).at(move.toSquare.row).has_value());
            m_grid[3][move.toSquare.row] = m_grid.at(0).at(move.toSquare.row);
            m_grid[0][move.toSquare.row] = std::nullopt;
        }
    }
    if (move.promotionTo.has_value()) {
        assert(piece.type == Piece::Type::Pawn);
        assert(move.toSquare.row == 0 || move.toSquare.row == 7);
        assert(move.promotionTo.value() != Piece::Type::Pawn);
        assert(move.promotionTo.value() != Piece::Type::King);
        m_grid[move.toSquare.col][move.toSquare.row] = Piece {move.promotionTo.value(), piece.color};
    }
}

void Board::setNextMoveColor(Color color) {
    m_nextMoveColor = color;
}

}
