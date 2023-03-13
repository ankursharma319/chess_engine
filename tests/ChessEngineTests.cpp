#include <gtest/gtest.h>
#include <glog/logging.h>
#include <unordered_map>

#include "ChessEngineLib/Engine.hpp"
#include "ChessEngineLib/Board.hpp"
#include "ChessEngineLib/Move.hpp"

GTEST_API_ int main(int argc, char **argv) {
    printf("Running main() from ChessEngineTests.cpp\n");
    // INFO=0, WARNING=1, ERROR=2, and FATAL=3
    FLAGS_stderrthreshold = 2; // also log to stderr, if only logging to file
    FLAGS_logtostderr = true; // instead of file
    FLAGS_minloglevel = 0; // lower is more logging
    FLAGS_v = 0; // applies to VLOG, higher is more logging
    google::InitGoogleLogging(argv[0]);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

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
            EXPECT_FALSE(board.grid()[c][r].has_value());
        }
    }

    for (int c = 0; c < 8; c++) {
        EXPECT_EQ(white_pawn, board.grid()[c][1].value());
        EXPECT_EQ(black_pawn, board.grid()[c][6].value());
    }

    EXPECT_EQ(black_rook, board.grid()[0][7].value());
    EXPECT_EQ(black_rook, board.grid()[7][7].value());

    EXPECT_EQ(white_rook, board.grid()[0][0].value());
    EXPECT_EQ(white_rook, board.grid()[7][0].value());

    EXPECT_EQ(black_knight, board.grid()[1][7].value());
    EXPECT_EQ(black_knight, board.grid()[6][7].value());

    EXPECT_EQ(white_knight, board.grid()[1][0].value());
    EXPECT_EQ(white_knight, board.grid()[6][0].value());

    EXPECT_EQ(black_bishop, board.grid()[2][7].value());
    EXPECT_EQ(black_bishop, board.grid()[5][7].value());

    EXPECT_EQ(white_bishop, board.grid()[2][0].value());
    EXPECT_EQ(white_bishop, board.grid()[5][0].value());

    EXPECT_EQ(black_queen, board.grid()[3][7].value());
    EXPECT_EQ(black_king, board.grid()[4][7].value());

    EXPECT_EQ(white_queen, board.grid()[3][0].value());
    EXPECT_EQ(white_king, board.grid()[4][0].value());

    EXPECT_EQ(Color::White, board.getNextMoveColor());
    EXPECT_TRUE(board.isCastlingAvailable(Color::White, Side::KingSide));
    EXPECT_TRUE(board.isCastlingAvailable(Color::White, Side::QueenSide));
    EXPECT_TRUE(board.isCastlingAvailable(Color::Black, Side::KingSide));
    EXPECT_TRUE(board.isCastlingAvailable(Color::Black, Side::QueenSide));

    EXPECT_EQ(0, board.getHalfMoveClock());
    EXPECT_EQ(1, board.getMoveNumber());
    EXPECT_FALSE(board.getEnPassantSquare().has_value());
}

TEST_F(EngineTestFixture, construct_correct_board_from_another_random_fen) {
    std::string fen = "rn1qkbnr/ppp2ppp/3p4/4p3/3PP1b1/5N2/PPP1BPPP/RNBQK2R b KQkq - 3 4";
    ASSERT_TRUE(Board::fromFen(fen).has_value());
    Board board = Board::fromFen(fen).value();

    ASSERT_TRUE(board.grid()[3][5].has_value());
    ASSERT_TRUE(board.grid()[4][4].has_value());
    ASSERT_TRUE(board.grid()[5][2].has_value());
    EXPECT_EQ(black_pawn, board.grid()[3][5].value());
    EXPECT_EQ(black_pawn, board.grid()[4][4].value());
    EXPECT_EQ(white_knight, board.grid()[5][2].value());

    ASSERT_TRUE(board.grid()[6][3].has_value());
    EXPECT_EQ(black_bishop, board.grid()[6][3].value());
    EXPECT_FALSE(board.grid()[7][4].has_value());

    EXPECT_EQ(Color::Black, board.getNextMoveColor());
    EXPECT_EQ(3, board.getHalfMoveClock());
    EXPECT_EQ(4, board.getMoveNumber());
    EXPECT_FALSE(board.getEnPassantSquare().has_value());
}

TEST_F(EngineTestFixture, generates_correct_legal_moves_for_each_square_starting_position) {
    Board board = Board::fromFen(starting_position_fen).value();
    std::unordered_set<Square> nonZeroMoveSquares = {
        {1,0}, {6,0}, {0,1}, {1,1}, {2,1}, {3,1}, {4,1}, {5,1}, {6,1}, {7,1}
    };
    std::unordered_map<Square, std::unordered_set<Square>> expectedDestinations = {
        {{1,0}, {{0,2},{2,2}}},
        {{6,0}, {{5,2},{7,2}}},
        {{0,1}, {{0,2},{0,3}}},
        {{1,1}, {{1,2},{1,3}}},
        {{4,1}, {{4,2},{4,3}}},
        {{7,1}, {{7,2},{7,3}}},
    };
    for (std::uint8_t col=0; col<8; col++) {
        for (std::uint8_t row=0; row<8; row++) {
            Square start_square = Square {col, row};
            std::unordered_set<Square> destinations = generateLegalDestinations(board, start_square);
            if (nonZeroMoveSquares.count(start_square)) {
                EXPECT_GT(destinations.size(), 0);
            } else {
                EXPECT_EQ(0, destinations.size());
            }
            if (expectedDestinations.count(start_square)) {
                EXPECT_EQ(expectedDestinations.at(start_square), destinations);
            }
        }
    }
}

TEST_F(EngineTestFixture, generates_correct_legal_moves_for_random_more_complex_position) {
    Board board = Board::fromFen("rn1qk2r/5ppp/4pn2/pPpp1b2/1b1P1B2/4PN1P/PP2KPP1/RN1Q1B1R w kq a6 0 9").value();
    EXPECT_TRUE(board.getEnPassantSquare().has_value());
    EXPECT_EQ((Square {0,5}), board.getEnPassantSquare().value());
    std::unordered_set<Square> nonZeroMoveSquares = {
        {1,0}, {3,0}, {7,0}, {0,1}, {1,1}, {6,1},
        {4,2}, {5,2}, {7,2}, {3,3}, {5,3}, {1,4}
    };
    std::unordered_map<Square, std::unordered_set<Square>> expectedDestinations = {
        {{1,0}, {{0,2},{2,2},{3,1}}}, // Knight
        {{3,0}, {{2,0},{4,0},{2,1},{3,1},{1,2},{3,2},{0,3}}}, //Queen
        {{7,0}, {{6,0},{7,1}}}, //Rook
        {{5,3}, {{7,1},{6,2},{4,4},{3,5},{2,6},{1,7},{6,4},{7,5}}}, // Bishop
        {{5,2}, {{3,1},{4,0},{4,4},{6,0},{6,4},{7,1},{7,3}}}, // Knight
        {{1,4}, {{1,5},{0,5}}}, //Pawn (enpassant)
        {{3,3}, {{2,4}}}, //Pawn (capture available,forward blocked)
        {{4,2}, {{4,3}}}, //Pawn
    };
    for (std::uint8_t col=0; col<8; col++) {
        for (std::uint8_t row=0; row<8; row++) {
            Square start_square = Square {col, row};
            std::unordered_set<Square> destinations = generateLegalDestinations(board, start_square);
            if (nonZeroMoveSquares.count(start_square)) {
                EXPECT_GT(destinations.size(), 0) << "for square " << start_square;
            } else {
                EXPECT_EQ(0, destinations.size()) << "for square " << start_square;
            }
            if (expectedDestinations.count(start_square)) {
                EXPECT_EQ(expectedDestinations.at(start_square), destinations) << "for square " << start_square;
            }
        }
    }
}
