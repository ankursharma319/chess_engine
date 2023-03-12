#ifndef CHESS_STRUCTS_HPP
#define CHESS_STRUCTS_HPP

#include <string>
#include <array>
#include <optional>

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

struct Move {
    Piece piece;
    Square fromSquare;
    Square toSquare;
};

}

#endif
