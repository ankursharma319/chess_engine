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

TEST_F(EngineTestFixture, generates_correct_legal_moves_for_king_includes_castling) {
    Board board = Board::fromFen("r3k2r/ppq1bppp/2nppn2/2p5/P3P1b1/1PNP1N2/1BPQBPPP/R3K2R b KQkq - 0 9").value();
    Square start_square = Square {4, 7};
    std::unordered_set<Square> expectedDestinations = {{{2,7},{3,7},{3,6},{5,7},{6,7}}};
    std::unordered_set<Square> destinations = generateLegalDestinations(board, start_square);
    EXPECT_EQ(expectedDestinations, destinations) << "for black king at square " << start_square;
}

TEST_F(EngineTestFixture, generates_correct_legal_moves_for_pawn_which_is_blocked_at_spawn) {
    Board board = Board::fromFen("r3k2r/ppq1bpp1/P1nppn2/2p5/4P3/1PNP1P1p/1BPQBP1P/R3K2R w KQkq - 0 13").value();
    Square start_square = Square {7, 1};
    std::unordered_set<Square> destinations = generateLegalDestinations(board, start_square);
    EXPECT_TRUE(destinations.empty()) << "for white pawn at square " << start_square;
}

TEST_F(EngineTestFixture, board_to_fen) {
    std::string fen1 = "rn1qk2r/5ppp/4pn2/pPpp1b2/1b1P1B2/4PN1P/PP2KPP1/RN1Q1B1R w kq a6 0 9";
    std::string fen2 = "rn1qk2r/5ppp/4pn2/pPpp1b2/1b1P1B2/4PN1P/PP2KPP1/RN1Q1B1R w kq a6 0 9";
    std::string fen3 = "rn1qk2r/5ppp/4pn2/pPpp1b2/1b1P1B2/4PN1P/PP2KP2/RN1Q1B1R w kq a6 0 9";
    std::string fen4 = "rn1qk2r/5ppp/4pn2/pPpp1b2/1b1P1B2/4PN1P/PP2KPP1/RN1Q1B1R b kq a6 0 9";
    std::string fen5 = "rn1qk2r/5ppp/4pn2/pPpp1b2/1b1P1B2/4PN1P/PP2KPP1/RN1Q1B1R w k a6 0 9";
    std::string fen6 = "rn1qk2r/5ppp/4pn2/pPpp1b2/1b1P1B2/4PN1P/PP2KPP1/RN1Q1B1R w kq a6 0 8";
    Board board1 = Board::fromFen(fen1).value();
    Board board2 = Board::fromFen(fen2).value();
    Board board3 = Board::fromFen(fen3).value();
    Board board4 = Board::fromFen(fen4).value();
    Board board5 = Board::fromFen(fen5).value();
    Board board6 = Board::fromFen(fen6).value();
    EXPECT_EQ(fen1, board1.fen());
    EXPECT_EQ(fen2, board2.fen());
    EXPECT_EQ(fen3, board3.fen());
    EXPECT_EQ(fen4, board4.fen());
    EXPECT_EQ(fen5, board5.fen());
    EXPECT_EQ(fen6, board6.fen());
}

TEST_F(EngineTestFixture, equality_operator_for_boards) {
    Board board1 = Board::fromFen("rn1qk2r/5ppp/4pn2/pPpp1b2/1b1P1B2/4PN1P/PP2KPP1/RN1Q1B1R w kq a6 0 9").value();
    Board board2 = Board::fromFen("rn1qk2r/5ppp/4pn2/pPpp1b2/1b1P1B2/4PN1P/PP2KPP1/RN1Q1B1R w kq a6 0 9").value();
    Board board3 = Board::fromFen("rn1qk2r/5ppp/4pn2/pPpp1b2/1b1P1B2/4PN1P/PP2KP2/RN1Q1B1R w kq a6 0 9").value();
    Board board4 = Board::fromFen("rn1qk2r/5ppp/4pn2/pPpp1b2/1b1P1B2/4PN1P/PP2KPP1/RN1Q1B1R b kq a6 0 9").value();
    Board board5 = Board::fromFen("rn1qk2r/5ppp/4pn2/pPpp1b2/1b1P1B2/4PN1P/PP2KPP1/RN1Q1B1R w k a6 0 9").value();
    Board board6 = Board::fromFen("rn1qk2r/5ppp/4pn2/pPpp1b2/1b1P1B2/4PN1P/PP2KPP1/RN1Q1B1R w kq a6 0 8").value();
    EXPECT_EQ(board1, board2);
    EXPECT_NE(board1, board3);
    EXPECT_NE(board1, board4);
    EXPECT_NE(board1, board5);
    EXPECT_NE(board1, board6);
}

TEST_F(EngineTestFixture, board_allows_making_legal_moves) {
    Board board = Board::fromFen("rn1qk2r/5ppp/4pn2/pPpp1b2/1b1P1B2/4PN1P/PP2KPP1/RN1Q1B1R w kq a6 0 9").value();
    Board board_init = board;
    std::unordered_map<Square, std::unordered_set<Square>> expectedValidDestinations = {
        {{1,0}, {{0,2},{2,2},{3,1}}}, // Knight
        {{3,0}, {{2,0},{4,0},{2,1},{3,1},{1,2},{3,2},{0,3}}}, //Queen
        {{7,0}, {{6,0},{7,1}}}, //Rook
        {{5,3}, {{7,1},{6,2},{4,4},{3,5},{2,6},{1,7},{6,4},{7,5}}}, // Bishop
        {{5,2}, {{3,1},{4,0},{4,4},{6,0},{6,4},{7,1},{7,3}}}, // Knight
        {{1,4}, {{1,5},{0,5}}}, //Pawn (enpassant)
        {{3,3}, {{2,4}}}, //Pawn (capture available,forward blocked)
        {{4,2}, {{4,3}}}, //Pawn
    };
    std::unordered_set<Square> exampleInvalidSources = {
        {0,0},{5,0},{5,1},{3,7},{4,7},{5,5},{0,4}, //occupied
        {2,0},{4,0},{7,1},{6,7},{4,4}, //empty
    };
    for (const auto& [src, dsts]: expectedValidDestinations) {
        Piece piece = board.at(src).value();
        for(Square dst: dsts) {
            Move move {piece, src, dst};
            EXPECT_TRUE(makeMove(board, move)) << " for move " << move;
            EXPECT_EQ(piece, board.at(dst).value());
            // en passant might still put some piece on dst
            if (piece.type != Piece::Type::King) {
                EXPECT_FALSE(board.at(src).has_value());
            }
            EXPECT_TRUE(board.at({0,0}).has_value());
            EXPECT_TRUE(board.at({7,7}).has_value());
            EXPECT_NE(board_init, board);
            board = board_init;
        }
    }
    for (const auto& src: exampleInvalidSources) {
        for (std::uint8_t col=0; col<8; col++) {
            for (std::uint8_t row=0; row<8; row++) {
                Square dst = Square {col, row};
                Piece piece = board.at(src).value_or(black_king);
                Move move {piece, src, dst};
                EXPECT_FALSE(makeMove(board, move));
                EXPECT_EQ(board_init, board);
            }
        }
    }
}

TEST_F(EngineTestFixture, fen_gets_updated_after_legal_move) {
    std::string fen = "rnbqk2r/ppppppbp/5np1/8/2PP3P/2N5/PP2PPP1/R1BQKBNR b KQkq - 0 4";
    Board board = Board::fromFen(fen).value();
    EXPECT_FALSE(makeMove(board, Move(black_rook, {4,7}, {6,7})));
    EXPECT_EQ(fen, board.fen());
    EXPECT_TRUE(makeMove(board, Move(black_king, {4,7}, {6,7})));
    EXPECT_EQ("rnbq1rk1/ppppppbp/5np1/8/2PP3P/2N5/PP2PPP1/R1BQKBNR w KQ - 1 5", board.fen());
}

