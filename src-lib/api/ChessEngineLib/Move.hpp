#ifndef CHESS_STRUCTS_HPP
#define CHESS_STRUCTS_HPP

#include <string>
#include <array>
#include <optional>
#include <ostream>
#include <functional>

namespace ChessEngineLib {

enum Color {
    Black, White
};

struct Square {
    std::uint8_t col;
    std::uint8_t row;

    bool operator==(Square other) const {
        return (col == other.col) && (row == other.row);
    }
};

inline std::ostream & operator<<(std::ostream &os, Square s) {
    os << "(" << +s.col << "," << +s.row << ")";
    return os;
}

struct Piece {
    enum Type {
        Pawn, Knight, Bishop, Rook, Queen, King
    };
    Piece (Type typ, Color col) {
        type = typ;
        color = col;
    }
    Type type;
    Color color;

    bool operator==(const Piece& other) const {
        return (type == other.type) && (color == other.color);
    }
};

inline std::ostream & operator<<(std::ostream &os, Color c) {
    switch (c) {
        case Color::Black:
            os << "Black";
            break;
        case Color::White:
            os << "White";
            break;
    }
    return os;
}

inline std::ostream & operator<<(std::ostream &os, Piece::Type t) {
    switch (t) {
        case Piece::Type::King:
            os << "King";
            break;
        case Piece::Type::Queen:
            os << "Queen";
            break;
        case Piece::Type::Rook:
            os << "Rook";
            break;
        case Piece::Type::Bishop:
            os << "Bishop";
            break;
        case Piece::Type::Knight:
            os << "Knight";
            break;
        case Piece::Type::Pawn:
            os << "Pawn";
            break;
    }
    return os;
}

inline std::ostream & operator<<(std::ostream &os, Piece const& p) {
    os << p.color << " " << p.type;
    return os;
}

struct Move {
    Piece piece;
    Square fromSquare;
    Square toSquare;
    std::optional<Piece::Type> promotionTo;
};

}

// custom specialization of std::hash can be injected in namespace std
template<>
struct std::hash<ChessEngineLib::Square>
{
    std::size_t operator()(ChessEngineLib::Square s) const noexcept
    {
        std::size_t h1 = std::hash<std::uint8_t>{}(s.col);
        std::size_t h2 = std::hash<std::uint8_t>{}(s.row);
        return h1 ^ (h2 << 1); // or use boost::hash_combine
    }
};

#endif
