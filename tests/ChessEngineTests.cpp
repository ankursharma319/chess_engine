#include <gtest/gtest.h>
#include "ChessEngineLib/Engine.hpp"
#include "ChessEngineLib/Board.hpp"

using namespace ChessEngineLib;

class EngineTestFixture : public ::testing::Test {
protected:
    EngineTestFixture() {
    }

    ~EngineTestFixture() = default;

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


TEST_F(EngineTestFixture, assert_validation_of_fen_works_correctly) {
	std::vector<std::string> invalidFens {
		"", "hello", "\n", "\twhat\n"
	};
	for (const auto& fen : invalidFens) {
		EXPECT_FALSE(validateFen(fen));
	}
	std::vector<std::string> validFens {
        starting_position_fen,
        "rn1qkbnr/ppp2ppp/3p4/4p3/3PP1b1/5N2/PPP1BPPP/RNBQK2R b KQkq - 3 4"
	};
	for (const auto& fen : validFens) {
		EXPECT_TRUE(validateFen(fen));
	}
}

TEST_F(EngineTestFixture, construct_correct_board_from_invalid_fens_should_fail) {
    std::vector<std::string> invalidFens {
		"", "hello", "\n", "\twhat\n"
	};
    for (const auto& fen : invalidFens) {
        EXPECT_FALSE(Board::fromFen(fen).has_value());
	}
}

TEST_F(EngineTestFixture, construct_correct_board_from_starting_position_fen) {
    auto optionalBoard = Board::fromFen(starting_position_fen);
    ASSERT_TRUE(optionalBoard.has_value());
    Board board = optionalBoard.value();

    for (int c = 0; c < 8; c++) {
        for (int r = 2; r < 6; r++) {
            EXPECT_FALSE(board.getContents()[c][r].has_value());
        }
    }

    for (int c = 0; c < 8; c++) {
        EXPECT_EQ(white_pawn, board.getContents()[c][1].value());
        EXPECT_EQ(black_pawn, board.getContents()[c][6].value());
    }

    EXPECT_EQ(black_rook, board.getContents()[0][7].value());
    EXPECT_EQ(black_rook, board.getContents()[7][7].value());

    EXPECT_EQ(white_rook, board.getContents()[0][0].value());
    EXPECT_EQ(white_rook, board.getContents()[7][0].value());

    EXPECT_EQ(black_knight, board.getContents()[1][7].value());
    EXPECT_EQ(black_knight, board.getContents()[6][7].value());

    EXPECT_EQ(white_knight, board.getContents()[1][0].value());
    EXPECT_EQ(white_knight, board.getContents()[6][0].value());

    EXPECT_EQ(black_bishop, board.getContents()[2][7].value());
    EXPECT_EQ(black_bishop, board.getContents()[5][7].value());

    EXPECT_EQ(white_bishop, board.getContents()[2][0].value());
    EXPECT_EQ(white_bishop, board.getContents()[5][0].value());

    EXPECT_EQ(black_queen, board.getContents()[3][7].value());
    EXPECT_EQ(black_king, board.getContents()[4][7].value());

    EXPECT_EQ(white_queen, board.getContents()[3][0].value());
    EXPECT_EQ(white_king, board.getContents()[4][0].value());

    EXPECT_EQ(Color::White, board.getNextMoveColor());
    EXPECT_FALSE(board.isCastlingExpired(Color::White, Side::KingSide));
    EXPECT_FALSE(board.isCastlingExpired(Color::White, Side::QueenSide));
    EXPECT_FALSE(board.isCastlingExpired(Color::Black, Side::KingSide));
    EXPECT_FALSE(board.isCastlingExpired(Color::Black, Side::QueenSide));

    EXPECT_EQ(0, board.getHalfMoveClock());
    EXPECT_EQ(0, board.getMoveNumber());
    EXPECT_FALSE(board.getEnPassantSquare().has_value());
}

TEST_F(EngineTestFixture, construct_correct_board_from_another_random_fen) {
    std::string fen = "rn1qkbnr/ppp2ppp/3p4/4p3/3PP1b1/5N2/PPP1BPPP/RNBQK2R b KQkq - 3 4";
    ASSERT_TRUE(Board::fromFen(fen).has_value());
    Board board = Board::fromFen(fen).value();

    ASSERT_TRUE(board.getContents()[6][3].has_value());
    EXPECT_EQ(black_bishop, board.getContents()[6][3].value());
    EXPECT_FALSE(board.getContents()[7][4].has_value());
}
