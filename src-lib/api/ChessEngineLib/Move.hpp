#ifndef CHESS_STRUCTS_HPP
#define CHESS_STRUCTS_HPP

#include <string>
#include <array>
#include <optional>
#include <ostream>

namespace ChessEngineLib {

enum Color {
    Black, White
};

struct Square {
    std::uint8_t col;
    std::uint8_t row;
};

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
};

}

#endif
