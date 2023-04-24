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
    char pgn_rank() const {
        return row + 1;
    }
    char pgn_file() const {
        return 'a' + col;
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

    bool operator!=(const Piece& other) const {
        return !(*this == other);
    }

    char fen_symbol() const {
        char c = 0;
        switch (type) {
            case Type::King:
                c = 'k';
                break;
            case Type::Queen:
                c = 'q';
                break;
            case Type::Rook:
                c = 'r';
                break;
            case Type::Bishop:
                c = 'b';
                break;
            case Type::Knight:
                c = 'n';
                break;
            case Type::Pawn:
                c = 'p';
                break;
        }
        if (color == Color::White) {
            c = toupper(c);
        }
        return c;
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
    Move() = delete;
    Move(Square from_square, Square to_square,
        std::optional<Piece::Type> const& promotion_to = std::nullopt
    ): fromSquare(from_square), toSquare(to_square), promotionTo(promotion_to)
    {}

    bool operator==(Move const& other) const {
        return
            (fromSquare == other.fromSquare) &&
            (toSquare == other.toSquare) &&
            (promotionTo == other.promotionTo);
    }

    Square fromSquare;
    Square toSquare;
    std::optional<Piece::Type> promotionTo {std::nullopt};
};


inline std::ostream & operator<<(std::ostream &os, Move const& m) {
    os << "from " << m.fromSquare << " to " << m.toSquare;
    return os;
}

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

template<>
struct std::hash<ChessEngineLib::Piece>
{
    std::size_t operator()(ChessEngineLib::Piece const& p) const noexcept
    {
        std::size_t h1 = std::hash<ChessEngineLib::Piece::Type>{}(p.type);
        std::size_t h2 = std::hash<ChessEngineLib::Color>{}(p.color);
        return h1 ^ (h2 << 1);
    }
};

template<>
struct std::hash<ChessEngineLib::Move>
{
    std::size_t operator()(ChessEngineLib::Move const& s) const noexcept
    {
        std::size_t h1 = std::hash<ChessEngineLib::Square>{}(s.toSquare);
        std::size_t h2 = std::hash<ChessEngineLib::Square>{}(s.fromSquare);
        std::size_t h3 = std::hash<ChessEngineLib::Piece::Type>{}(
            s.promotionTo.value_or(ChessEngineLib::Piece::Type::King)
        );
        return h1 ^ ((h2 ^ (h3 << 1)) << 1);
    }
};

#endif
