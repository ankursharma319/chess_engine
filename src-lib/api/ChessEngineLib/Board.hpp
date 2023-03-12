#ifndef BOARD_HPP
#define BOARD_HPP

#include <string>
#include <array>
#include <optional>

#include "Move.hpp"

namespace ChessEngineLib {

enum Side {
    QueenSide, KingSide
};

class Board {
public:
    static std::optional<Board> fromFen(std::string const& fen);
    ~Board() = default;

    using Board2dArray = std::array<std::array<std::optional<Piece>, 8>, 8>;
    Board2dArray getContents() const;
    Color getNextMoveColor() const;
    bool isCastlingAvailable(Color color, Side side) const;
    std::size_t getHalfMoveClock() const; //For fifty move rule
    std::size_t getMoveNumber() const; // Starts at 1
    std::optional<Square> getEnPassantSquare(); // Just behind the pawn that moved 2 squares

private:
    struct CastlingAvailability {
        bool whiteQueenSide {false};
        bool whiteKingSide {false};
        bool blackQueenSide {false};
        bool blackKingSide {false};
    };

    Board() = default;
    static std::optional<CastlingAvailability> parse_castling_availability(std::string const& fen_chunk);

    Board2dArray m_grid;
    Color m_nextMoveColor {Color::Black};
    CastlingAvailability m_castlingAvailability {};
    std::size_t m_halfMoveClock {0};
    std::size_t m_moveNumber {0};
    std::optional<Square> m_enPassantSquare {std::nullopt};
};

}

#endif
