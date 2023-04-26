#include <gtest/gtest.h>
#include <glog/logging.h>

#include "ChessEngineLib/Game.hpp"
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
};


TEST_F(AiPlayerTestFixture, random_move_player_generates_moves) {
    RandomMovePlayer rmp = RandomMovePlayer();
    Game game {};
    std::optional<Move> move_opt = rmp.getMove(game.board());
    ASSERT_TRUE(move_opt.has_value());
    EXPECT_TRUE(
        (white_pawn == game.board().at(move_opt.value().fromSquare)) ||
        (white_knight == game.board().at(move_opt.value().fromSquare))
    );
    EXPECT_TRUE(game.makeMove(move_opt.value()));
    move_opt = rmp.getMove(game.board());
    ASSERT_TRUE(move_opt.has_value());
    EXPECT_TRUE(
        (black_pawn == game.board().at(move_opt.value().fromSquare)) ||
        (black_knight == game.board().at(move_opt.value().fromSquare))
    );
}

TEST_F(AiPlayerTestFixture, random_move_player_can_play_long_games) {
    RandomMovePlayer rmp = RandomMovePlayer();
    Game game {};
    while (!game.result().has_value()) {
        VLOG(1) << "Retrieving move#" << game.movesSize() << " for board " << game.board();
        std::optional<Move> move_opt = rmp.getMove(game.board());
        VLOG(1) << "Retrieved move#" << game.movesSize() << " for board " << game.board();
        if(!move_opt.has_value()) {
            break;
        }
        VLOG(1) << "Making move#" << game.movesSize() + 1;
        game.makeMove(move_opt.value());
        VLOG(1) << "Finished making move#" << game.movesSize() + 1;
    }
    LOG(INFO) << "Played the following game using random moves only";
    LOG(INFO) << game.toPgn();
}
