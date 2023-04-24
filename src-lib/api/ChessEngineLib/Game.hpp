#ifndef GAME_HPP
#define GAME_HPP

#include <string>
#include <vector>
#include <ostream>

#include "Board.hpp"
#include "Move.hpp"
#include "GameEngine.hpp"

namespace ChessEngineLib {

class Game {

public:
    Game();
    static std::optional<Game> fromPgn(std::string const& pgn);

    struct SevenTagRoster {
        std::string event;
        std::string site;
        std::string date;
        std::string round;
        std::string white;
        std::string black;
        std::optional<ResultType> result;
    };
    struct MoveWithContext {
        MoveWithContext(
            Move const& move,
            Piece const& piece,
            bool is_capture,
            bool is_check,
            bool is_checkmate,
            bool is_src_file_ambig,
            bool is_src_rank_ambig,
            std::optional<Side> is_castle
        );

        Move move;
        Piece piece;
        bool isCapture;
        bool isCheck;
        bool isCheckmate;
        bool isSrcFileAmbigious;
        bool isSrcRankAmbigious;
        std::optional<Side> isCastle;

        bool operator==(MoveWithContext const& other) const {
            return
                (move == other.move) &&
                (piece == other.piece) &&
                (isCapture == other.isCapture) &&
                (isCheck == other.isCheck) &&
                (isCheckmate == other.isCheckmate) &&
                (isSrcFileAmbigious == other.isSrcFileAmbigious) &&
                (isSrcRankAmbigious == other.isSrcRankAmbigious) &&
                (isCastle == other.isCastle);
        }
    };

    std::string toPgn() const;
    SevenTagRoster const& sevenTagRoster() const;
    std::optional<ResultType> result() const;

    std::optional<MoveWithContext> moveAt(std::size_t moveNum, Color color) const;
    std::optional<MoveWithContext> moveAt(std::size_t halfMoveNum) const;
    std::size_t movesSize() const;
    Board const& board() const;

private:
    SevenTagRoster roster_;
    std::vector<MoveWithContext> moves_;
    std::optional<ResultType> result_;
    Board board_;
};

}

#endif
