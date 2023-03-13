#include "Board.hpp"
#include "Move.hpp"

#include <nlohmann/json.hpp>
#include <glog/logging.h>

#include <cstdint>
#include <optional>

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

}

namespace ChessEngineLib {

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

std::optional<Square> Board::getEnPassantSquare() {
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

}
