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
    static std::optional<Board> fromFen(std::string fen);
    ~Board() = default;

    std::array<std::array<std::optional<Piece>, 8>, 8> getContents() const;
    Color getNextMoveColor() const;
    bool isCastlingExpired(Color color, Side side) const;
    unsigned short getHalfMoveClock() const; //For fifty move rule
    std::size_t getMoveNumber() const; // Starts at 1
    std::optional<Square> getEnPassantSquare(); // Just behind the pawn that moved 2 squares

private:
    Board();
    std::array<std::array<std::optional<Piece>, 8>, 8> m_grid;
    Color m_nextMoveColor {Color::Black};
    bool m_whiteCastlingQueenSideExpired {false};
    bool m_whiteCastlingKingSideExpired {false};
    bool m_blackCastlingQueenSideExpired {false};
    bool m_blackCastlingKingSideExpired {false};
    unsigned short m_halfMoveClock {0};
    std::size_t m_moveNumber {0};
    std::optional<Square> m_enPassantSquare {std::nullopt};
};

}

#endif
