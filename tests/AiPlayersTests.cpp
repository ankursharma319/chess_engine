#include <gtest/gtest.h>
#include <glog/logging.h>

#include "ChessEngineLib/GameEngine.hpp"
#include "ChessEngineLib/Board.hpp"
#include "ChessEngineLib/Move.hpp"
#include "ChessEngineLib/RandomMovePlayer.hpp"

using namespace ChessEngineLib;

class AiPlayerTestFixture : public ::testing::Test {
protected:
    AiPlayerTestFixture() {
    }

    ~AiPlayerTestFixture() = default;

    const Piece black_pawn = Piece(Piece::Type::Pawn, Color::Black);
    const Piece black_bishop = Piece(Piece::Type::Bishop, Color::Black);
    const Piece black_knight = Piece(Piece::Type::Knight, Color::Black);
    const Piece black_rook = Piece(Piece::Type::Rook, Color::Black);
    const Piece black_queen = Piece(Piece::Type::Queen, Color::Black);
    const Piece black_king = Piece(Piece::Type::King, Color::Black);

    const Piece white_pawn = Piece(Piece::Type::Pawn, Color::White);
    const Piece white_bishop = Piece(Piece::Type::Bishop, Color::White);
    const Piece white_knight = Piece(Piece::Type::Knight, Color::White);
    const Piece white_rook = Piece(Piece::Type::Rook, Color::White);
    const Piece white_queen = Piece(Piece::Type::Queen, Color::White);
    const Piece white_king = Piece(Piece::Type::King, Color::White);

    const std::string starting_position_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
};


TEST_F(AiPlayerTestFixture, random_move_player_generates_moves) {
    RandomMovePlayer rmp = RandomMovePlayer();
    Board board = Board::fromFen(starting_position_fen).value();
    std::optional<Move> move_opt = rmp.getMove(board);
    ASSERT_TRUE(move_opt.has_value());
    EXPECT_TRUE(
        (white_pawn == move_opt.value().piece) ||
        (white_knight == move_opt.value().piece)
    );
    EXPECT_TRUE(makeMove(board, move_opt.value()));
    move_opt = rmp.getMove(board);
    ASSERT_TRUE(move_opt.has_value());
    EXPECT_TRUE(
        (black_pawn == move_opt.value().piece) ||
        (black_knight == move_opt.value().piece)
    );
}

