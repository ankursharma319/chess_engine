#ifndef BOARD_HPP
#define BOARD_HPP

#include <string>
#include <array>
#include <optional>
#include <unordered_map>

#include "Move.hpp"

namespace ChessEngineLib {

enum Side {
    QueenSide, KingSide
};

class Board {
public:
    static std::optional<Board> fromFen(std::string const& fen);
    static Board startingPosBoard();
    ~Board() = default;

    using Board2dArray = std::array<std::array<std::optional<Piece>, 8>, 8>;
    Board2dArray const& grid() const;
    std::optional<Piece> const& at(Square square) const;
    Color getNextMoveColor() const;
    bool isCastlingAvailable(Color color, Side side) const;
    std::size_t getHalfMoveClock() const; //For fifty move rule
    std::size_t getMoveNumber() const; // Starts at 1
    std::optional<Square> getEnPassantSquare() const; // Just behind the pawn that moved 2 squares

    std::string fen() const;

    // useful for encoding repetitions for threefold repetition
    std::string fenWithoutMoveNumbers() const;

    bool operator==(const Board& other) const;
    bool operator!=(const Board& other) const;

    void forceMakeMove(Move const& move);
    void setNextMoveColor(Color color);

private:
    struct CastlingAvailability {
        bool whiteQueenSide {true};
        bool whiteKingSide {true};
        bool blackQueenSide {true};
        bool blackKingSide {true};
        bool operator==(const CastlingAvailability& other) const;
    };

    Board() = default;
    static std::optional<CastlingAvailability> parse_castling_availability(std::string const& fen_chunk);
    void expireCastlingAvailability(Color color, Side side);

    Board2dArray m_grid;
    Color m_nextMoveColor {Color::Black};
    CastlingAvailability m_castlingAvailability {};
    std::size_t m_halfMoveClock {0};
    std::size_t m_moveNumber {0};
    std::optional<Square> m_enPassantSquare {std::nullopt};
};

std::ostream & operator<<(std::ostream &os, Board const& b);

}

#endif
