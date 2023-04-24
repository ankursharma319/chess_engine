#include <gtest/gtest.h>
#include <glog/logging.h>

#include "ChessEngineLib/Game.hpp"
#include "ChessEngineLib/GameEngine.hpp"
#include "ChessEngineLib/Board.hpp"
#include "ChessEngineLib/Move.hpp"

using namespace ChessEngineLib;

class GameTestFixture : public ::testing::Test {
protected:
    GameTestFixture() {
    }

    ~GameTestFixture() = default;

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


TEST_F(GameTestFixture, parsing_normal_pgn) {
    std::string const pgn = R"raw(
[Event "F/S Return Match"]
[Site "Belgrade, Serbia JUG"]
[Date "1992.11.04"]
[Round "29"]
[White "Fischer, Robert J."]
[Black "Spassky, Boris V."]
[Result "1/2-1/2"]

1. e4 e5 2. Nf3 Nc6 3. Bb5 a6 {This opening is called the Ruy Lopez.}
4. Ba4 Nf6 5. O-O Be7 6. Re1 b5 7. Bb3 d6 8. c3 O-O 9. h3 Nb8 10. d4 Nbd7
11. c4 c6 12. cxb5 axb5 13. Nc3 Bb7 14. Bg5 b4 15. Nb1 h6 16. Bh4 c5 17. dxe5
Nxe4 18. Bxe7 Qxe7 19. exd6 Qf6 20. Nbd2 Nxd6 21. Nc4 Nxc4 22. Bxc4 Nb6
23. Ne5 Rae8 24. Bxf7+ Rxf7 25. Nxf7 Rxe1+ 26. Qxe1 Kxf7 27. Qe3 Qg5 28. Qxg5
hxg5 29. b3 Ke6 30. a3 Kd6 31. axb4 cxb4 32. Ra5 Nd5 33. f3 Bc8 34. Kf2 Bf5
35. Ra7 g6 36. Ra6+ Kc5 37. Ke1 Nf4 38. g3 Nxh3 39. Kd2 Kb5 40. Rd6 Kc5 41. Ra6
Nf2 42. g4 Bd3 43. Re6 1/2-1/2
    )raw";

    std::optional<Game> game_opt = Game::fromPgn(pgn);
    ASSERT_TRUE(game_opt.has_value());
    Game game = game_opt.value();
    ASSERT_EQ("1992.11.04", game.sevenTagRoster().date);
    ASSERT_EQ("Belgrade, Serbia JUG", game.sevenTagRoster().site);
    ASSERT_EQ("29", game.sevenTagRoster().round);
    ASSERT_EQ("F/S Return Match", game.sevenTagRoster().event);
    ASSERT_EQ("Fischer, Robert J.", game.sevenTagRoster().white);
    ASSERT_EQ("Spassky, Boris V.", game.sevenTagRoster().black);
    ASSERT_EQ(ResultType::Draw, game.sevenTagRoster().result.value());
    VLOG(2) << "Tests for seven tag roster passing";

    ASSERT_EQ(ResultType::Draw, game.result().value());
    ASSERT_EQ(85, game.movesSize());
    ASSERT_EQ("8/8/4R1p1/2k3p1/1p4P1/1P1b1P2/3K1n2/8 b - - 2 43", game.board().fen());

    VLOG(2) << "Starting tests to make sure both moveAt funcs are consistent";
    ASSERT_EQ(game.moveAt(1).value(), game.moveAt(1, Color::White)) << "moveAt 1";
    ASSERT_EQ(game.moveAt(2).value(), game.moveAt(1, Color::Black)) << "moveAt 2";
    ASSERT_EQ(game.moveAt(5).value(), game.moveAt(3, Color::White));
    ASSERT_EQ(game.moveAt(10).value(), game.moveAt(5, Color::Black));

    VLOG(2) << "Starting tests for move #1";
    ASSERT_EQ(Move({{4,1}, {4, 3}}), game.moveAt(1).value().move);
    ASSERT_EQ(white_pawn, game.moveAt(1).value().piece);
    ASSERT_FALSE(game.moveAt(1).value().isCheck);
    ASSERT_FALSE(game.moveAt(1).value().isCapture);
    ASSERT_FALSE(game.moveAt(1).value().isCheckmate);
    ASSERT_FALSE(game.moveAt(1).value().isSrcFileAmbigious);
    ASSERT_FALSE(game.moveAt(1).value().isSrcRankAmbigious);
    ASSERT_FALSE(game.moveAt(1).value().isCastle.has_value());

    VLOG(2) << "Starting tests for move #2";
    ASSERT_EQ(Move({{4,6}, {4, 4}}), game.moveAt(2).value().move);
    ASSERT_EQ(black_pawn, game.moveAt(2).value().piece);
    ASSERT_FALSE(game.moveAt(2).value().isCheck);
    ASSERT_FALSE(game.moveAt(2).value().isCapture);
    ASSERT_FALSE(game.moveAt(2).value().isCheckmate);
    ASSERT_FALSE(game.moveAt(2).value().isSrcFileAmbigious);
    ASSERT_FALSE(game.moveAt(2).value().isSrcRankAmbigious);
    ASSERT_FALSE(game.moveAt(2).value().isCastle.has_value());

    VLOG(2) << "Starting tests for move #9";
    ASSERT_EQ(Move({{4,0}, {6, 0}}), game.moveAt(9).value().move);
    ASSERT_EQ(white_king, game.moveAt(9).value().piece);
    ASSERT_FALSE(game.moveAt(9).value().isCheck);
    ASSERT_FALSE(game.moveAt(9).value().isCapture);
    ASSERT_FALSE(game.moveAt(9).value().isCheckmate);
    ASSERT_FALSE(game.moveAt(9).value().isSrcFileAmbigious);
    ASSERT_FALSE(game.moveAt(9).value().isSrcRankAmbigious);
    ASSERT_EQ(Side::KingSide, game.moveAt(9).value().isCastle.value());

    ASSERT_EQ(Move({{1,7}, {3, 6}}), game.moveAt(20).value().move);
    ASSERT_EQ(black_knight, game.moveAt(20).value().piece);
    ASSERT_FALSE(game.moveAt(20).value().isCheck);
    ASSERT_FALSE(game.moveAt(20).value().isCapture);
    ASSERT_FALSE(game.moveAt(20).value().isCheckmate);
    ASSERT_TRUE(game.moveAt(20).value().isSrcFileAmbigious);
    ASSERT_FALSE(game.moveAt(20).value().isSrcRankAmbigious);
    ASSERT_FALSE(game.moveAt(20).value().isCastle.has_value());

    ASSERT_EQ(Move({{1,2}, {2, 3}}), game.moveAt(43).value().move);
    ASSERT_EQ(white_bishop, game.moveAt(43).value().piece);
    ASSERT_FALSE(game.moveAt(43).value().isCheck);
    ASSERT_TRUE(game.moveAt(43).value().isCapture);
    ASSERT_FALSE(game.moveAt(43).value().isCheckmate);
    ASSERT_FALSE(game.moveAt(43).value().isSrcFileAmbigious);
    ASSERT_FALSE(game.moveAt(43).value().isSrcRankAmbigious);
    ASSERT_FALSE(game.moveAt(43).value().isCastle.has_value());

    ASSERT_EQ(Move({{4,7}, {4, 0}}), game.moveAt(50).value().move);
    ASSERT_EQ(black_rook, game.moveAt(50).value().piece);
    ASSERT_TRUE(game.moveAt(50).value().isCheck);
    ASSERT_TRUE(game.moveAt(50).value().isCapture);
    ASSERT_FALSE(game.moveAt(50).value().isCheckmate);
    ASSERT_FALSE(game.moveAt(50).value().isSrcFileAmbigious);
    ASSERT_FALSE(game.moveAt(50).value().isSrcRankAmbigious);
    ASSERT_FALSE(game.moveAt(50).value().isCastle.has_value());

    std::string expected_pgn = R"raw([Event "F/S Return Match"]
[Site "Belgrade, Serbia JUG"]
[Date "1992.11.04"]
[Round "29"]
[White "Fischer, Robert J."]
[Black "Spassky, Boris V."]
[Result "1/2-1/2"]

1. e4 e5 2. Nf3 Nc6 3. Bb5 a6 4. Ba4 Nf6 5. O-O Be7 6. Re1 b5 7. Bb3 d6 8. c3 O-O 9. h3 Nb8 10. d4 Nbd7
11. c4 c6 12. cxb5 axb5 13. Nc3 Bb7 14. Bg5 b4 15. Nb1 h6 16. Bh4 c5 17. dxe5 Nxe4 18. Bxe7 Qxe7 19. exd6 Qf6 20. Nbd2 Nxd6
21. Nc4 Nxc4 22. Bxc4 Nb6 23. Ne5 Rae8 24. Bxf7+ Rxf7 25. Nxf7 Rxe1+ 26. Qxe1 Kxf7 27. Qe3 Qg5 28. Qxg5 hxg5 29. b3 Ke6 30. a3 Kd6
31. axb4 cxb4 32. Ra5 Nd5 33. f3 Bc8 34. Kf2 Bf5 35. Ra7 g6 36. Ra6+ Kc5 37. Ke1 Nf4 38. g3 Nxh3 39. Kd2 Kb5 40. Rd6 Kc5
41. Ra6 Nf2 42. g4 Bd3 43. Re6 1/2-1/2
)raw";
    ASSERT_EQ(expected_pgn, game.toPgn());
}


TEST_F(GameTestFixture, parsing_pgn_with_promotion_and_checkmate) {
    std::string const pgn = R"raw(
[Event "Rated Blitz game"]
[Site "https://lichess.org/89893Phw"]
[Date "2023.04.04"]
[White "DESTROYER7777777"]
[Black "kenkons"]
[Result "1-0"]
[UTCDate "2023.04.04"]
[UTCTime "17:13:40"]
[WhiteElo "2772"]
[BlackElo "2808"]
[WhiteRatingDiff "+6"]
[BlackRatingDiff "-6"]
[Variant "Standard"]
[TimeControl "180+0"]
[ECO "E61"]
[Opening "King's Indian Defense: Smyslov Variation"]
[Termination "Normal"]
[Annotator "lichess.org"]

1. d4 Nf6 2. c4 d6 3. Nf3 g6 4. Nc3 Bg7 5. Bg5 { E61 King's Indian Defense: Smyslov Variation } h6 6. Bh4 g5 7. Bg3 Nh5
8. e3 c5 9. d5 Bxc3+ 10. bxc3 Kd7 11. Ne5+ dxe5 12. Bxe5 Qa5 13. Bxh8 f6 14. Rc1 Qd8 15. Qxh5 Qxh8 16. Be2 Kc7 17. h4 Qg7
18. hxg5 hxg5 19. Qh7 Qxh7 20. Rxh7 Kd6 21. Rh8 Nd7 22. Bg4 b6 23. Kd2 Rb8 24. a4 Ba6 25. Rxb8 Nxb8 26. Kd3 e5
27. Rh1 e4+ 28. Kxe4 Bxc4 29. Be6 Bb3 30. Kf5 Bxa4 31. Kxf6 Bc2 32. Rh7 Bxh7 33. f3 a5 34. e4 Nd7+ 35. Bxd7 Kxd7
36. e5 Bg8 37. e6+ Kd6 38. e7 Kd7 39. d6 a4 40. Kg7 Bh7 41. Kf7 Bg6+ 42. Kxg6 a3 43. Kf7 a2 44. e8=Q+ Kxd6
45. Qe6+ Kc7 46. Qxa2 Kc6 47. Qe6+ Kb5 48. Qd5 Ka5 49. Qc4 b5 50. Qxc5 Ka4 51. Qb4# { White wins by checkmate. } 1-0
    )raw";

    std::optional<Game> game_opt = Game::fromPgn(pgn);
    ASSERT_TRUE(game_opt.has_value());
    Game game = game_opt.value();

    ASSERT_EQ(101, game.movesSize());
    ASSERT_EQ("8/5K2/8/1p4p1/kQ6/2P2P2/6P1/8 b - - 2 51", game.board().fen());

    ASSERT_EQ(Move({{4,6}, {4, 7}, std::make_optional(Piece::Type::Queen)}), game.moveAt(87).value().move);
    ASSERT_EQ(white_pawn, game.moveAt(87).value().piece);
    ASSERT_TRUE(game.moveAt(87).value().isCheck);
    ASSERT_FALSE(game.moveAt(87).value().isCapture);
    ASSERT_FALSE(game.moveAt(87).value().isCheckmate);
    ASSERT_FALSE(game.moveAt(87).value().isSrcFileAmbigious);
    ASSERT_FALSE(game.moveAt(87).value().isSrcRankAmbigious);
    ASSERT_FALSE(game.moveAt(87).value().isCastle.has_value());

    ASSERT_EQ(Move({{2,4}, {1, 3}}), game.moveAt(101).value().move);
    ASSERT_EQ(white_queen, game.moveAt(101).value().piece);
    ASSERT_FALSE(game.moveAt(101).value().isCheck);
    ASSERT_FALSE(game.moveAt(101).value().isCapture);
    ASSERT_TRUE(game.moveAt(101).value().isCheckmate);
    ASSERT_FALSE(game.moveAt(101).value().isSrcFileAmbigious);
    ASSERT_FALSE(game.moveAt(101).value().isSrcRankAmbigious);
    ASSERT_FALSE(game.moveAt(101).value().isCastle.has_value());

    ASSERT_EQ(ResultType::WhiteWin, game.result().value());
    std::string expected_pgn = R"raw([Event "Rated Blitz game"]
[Site "https://lichess.org/89893Phw"]
[Date "2023.04.04"]
[Round ""]
[White "DESTROYER7777777"]
[Black "kenkons"]
[Result "1-0"]

1. d4 Nf6 2. c4 d6 3. Nf3 g6 4. Nc3 Bg7 5. Bg5 h6 6. Bh4 g5 7. Bg3 Nh5 8. e3 c5 9. d5 Bxc3+ 10. bxc3 Kd7
11. Ne5+ dxe5 12. Bxe5 Qa5 13. Bxh8 f6 14. Rc1 Qd8 15. Qxh5 Qxh8 16. Be2 Kc7 17. h4 Qg7 18. hxg5 hxg5 19. Qh7 Qxh7 20. Rxh7 Kd6
21. Rh8 Nd7 22. Bg4 b6 23. Kd2 Rb8 24. a4 Ba6 25. Rxb8 Nxb8 26. Kd3 e5 27. Rh1 e4+ 28. Kxe4 Bxc4 29. Be6 Bb3 30. Kf5 Bxa4
31. Kxf6 Bc2 32. Rh7 Bxh7 33. f3 a5 34. e4 Nd7+ 35. Bxd7 Kxd7 36. e5 Bg8 37. e6+ Kd6 38. e7 Kd7 39. d6 a4 40. Kg7 Bh7
41. Kf7 Bg6+ 42. Kxg6 a3 43. Kf7 a2 44. e8=Q+ Kxd6 45. Qe6+ Kc7 46. Qxa2 Kc6 47. Qe6+ Kb5 48. Qd5 Ka5 49. Qc4 b5 50. Qxc5 Ka4
51. Qb4# 1-0
)raw";
    ASSERT_EQ(expected_pgn, game.toPgn());
}

TEST_F(GameTestFixture, parsing_pgn_with_ambigious_rank) {
    std::string const pgn = R"raw(
1. d4 d5 2. Nf3 Nf6 3. Nc3 Na6 4. Bd2 Bd7 5. Na4 Rb8 6. Ne5 Qc8 7. Nc5 g6 8. Ncd3 Ng8 9. f3 Kd8 10. Nf2 Ke8 11. Nc4 e6 12. Nd3 Ra8 13. Na5 Be7 14. Nc1 f6 15. a4 g5 16. Na2 Rb8 17. Nb4 Ra8 18. Nxd5 Nh6 19. Ne3 Rb8 20. c3 Ra8 21. Nc2 Rb8 22. Na3 Kf7 23. N3c4
    )raw";

    std::optional<Game> game_opt = Game::fromPgn(pgn);
    ASSERT_TRUE(game_opt.has_value());
    Game game = game_opt.value();

    ASSERT_EQ(45, game.movesSize());
    ASSERT_EQ("1rq4r/pppbbk1p/n3pp1n/N5p1/P1NP4/2P2P2/1P1BP1PP/R2QKB1R b KQ - 6 23", game.board().fen());

    ASSERT_EQ(Move({0,2}, {2, 3}), game.moveAt(45).value().move);
    ASSERT_EQ(white_knight, game.moveAt(45).value().piece);
    ASSERT_FALSE(game.moveAt(45).value().isCheck);
    ASSERT_FALSE(game.moveAt(45).value().isCapture);
    ASSERT_FALSE(game.moveAt(45).value().isCheckmate);
    ASSERT_FALSE(game.moveAt(45).value().isSrcFileAmbigious);
    ASSERT_TRUE(game.moveAt(45).value().isSrcRankAmbigious);
    ASSERT_FALSE(game.moveAt(45).value().isCastle.has_value());

    std::string expected_pgn = R"raw([Event ""]
[Site ""]
[Date ""]
[Round ""]
[White ""]
[Black ""]
[Result "*"]

1. d4 d5 2. Nf3 Nf6 3. Nc3 Na6 4. Bd2 Bd7 5. Na4 Rb8 6. Ne5 Qc8 7. Nc5 g6 8. Ncd3 Ng8 9. f3 Kd8 10. Nf2 Ke8
11. Nc4 e6 12. Nd3 Ra8 13. Na5 Be7 14. Nc1 f6 15. a4 g5 16. Na2 Rb8 17. Nb4 Ra8 18. Nxd5 Nh6 19. Ne3 Rb8 20. c3 Ra8
21. Nc2 Rb8 22. Na3 Kf7 23. N3c4
)raw";
    ASSERT_EQ(expected_pgn, game.toPgn());
}

TEST_F(GameTestFixture, parsing_pgn_with_ambigious_rank_and_file_and_check_checkmate_capture_together) {
    std::string const pgn = R"raw(
1. d4 e5 2. dxe5 d6 3. Bf4 Nf6 4. exf6 Qxf6 5. Nf3 Bf5 6. e4 Be7 7. Nc3 Nd7 8. exf5 Qxf5 9. Bd3 Ne5 10. Bxf5 d5 11. Nxe5 c5 12. Nxd5 Rd8 13. Nxe7 Kxe7 14. O-O b5 15. Nc6+ Kf6 16. Nxd8 Rxd8 17. Qxd8+ Kxf5 18. Qh8 Kxf4 19. Qxh7 Ke5 20. Qxg7+ Ke6 21. Qf8 Kf6 22. Qa8 c4 23. Rfe1 Kg7 24. Qxa7 Kf6 25. Qc5 Kg7 26. Qxb5 Kf6 27. Qxc4 Kg7 28. a4 Kf6 29. a5 Kg7 30. a6 Kf6 31. a7 Kg7 32. b4 Kf6 33. b5 Kg7 34. b6 Kf6 35. Qb3 Kg7 36. c4 Kf6 37. b7 Kg7 38. c5 Kf6 39. Re6+ fxe6 40. Re1 Ke7 41. b8=Q Kf6 42. a8=Q Ke7 43. c6 Kf6 44. c7 Ke7 45. c8=Q Kf6 46. Q8g3 Ke7 47. Qca6 Kd7 48. Qh8 Ke7 49. Qh6 Kd7 50. Qab5+ Ke7 51. Qbc3 Kf7 52. Rf1 Ke7 53. Qge3 Kf7 54. Qb8 Ke7 55. Qbc8 Kf7 56. Qhh8 Ke7 57. h3 e5 58. Qa8 Ke6 59. Qab8 Kf5 60. Qbc7 Ke6 61. Qhg7 Kf5 62. Qc3xe5#
    )raw";

    std::optional<Game> game_opt = Game::fromPgn(pgn);
    ASSERT_TRUE(game_opt.has_value());
    Game game = game_opt.value();

    ASSERT_EQ(123, game.movesSize());
    ASSERT_EQ("8/2Q3Q1/8/4Qk2/8/4Q2P/5PP1/5RK1 b - - 0 62", game.board().fen());

    VLOG(2) << "Making assertions about the last move";
    ASSERT_TRUE(game.moveAt(123).has_value());
    ASSERT_EQ(Move({2,2}, {4, 4}), game.moveAt(123).value().move);
    ASSERT_EQ(white_queen, game.moveAt(123).value().piece);
    ASSERT_FALSE(game.moveAt(123).value().isCheck);
    ASSERT_TRUE(game.moveAt(123).value().isCapture);
    ASSERT_TRUE(game.moveAt(123).value().isCheckmate);
    ASSERT_TRUE(game.moveAt(123).value().isSrcFileAmbigious);
    ASSERT_TRUE(game.moveAt(123).value().isSrcRankAmbigious);
    ASSERT_FALSE(game.moveAt(123).value().isCastle.has_value());

    VLOG(2) << "Making assertion about the result";
    ASSERT_EQ(ResultType::WhiteWin, game.result().value());

    std::string expected_pgn = R"raw([Event ""]
[Site ""]
[Date ""]
[Round ""]
[White ""]
[Black ""]
[Result "1-0"]

1. d4 e5 2. dxe5 d6 3. Bf4 Nf6 4. exf6 Qxf6 5. Nf3 Bf5 6. e4 Be7 7. Nc3 Nd7 8. exf5 Qxf5 9. Bd3 Ne5 10. Bxf5 d5
11. Nxe5 c5 12. Nxd5 Rd8 13. Nxe7 Kxe7 14. O-O b5 15. Nc6+ Kf6 16. Nxd8 Rxd8 17. Qxd8+ Kxf5 18. Qh8 Kxf4 19. Qxh7 Ke5 20. Qxg7+ Ke6
21. Qf8 Kf6 22. Qa8 c4 23. Rfe1 Kg7 24. Qxa7 Kf6 25. Qc5 Kg7 26. Qxb5 Kf6 27. Qxc4 Kg7 28. a4 Kf6 29. a5 Kg7 30. a6 Kf6
31. a7 Kg7 32. b4 Kf6 33. b5 Kg7 34. b6 Kf6 35. Qb3 Kg7 36. c4 Kf6 37. b7 Kg7 38. c5 Kf6 39. Re6+ fxe6 40. Re1 Ke7
41. b8=Q Kf6 42. a8=Q Ke7 43. c6 Kf6 44. c7 Ke7 45. c8=Q Kf6 46. Q8g3 Ke7 47. Qca6 Kd7 48. Qh8 Ke7 49. Qh6 Kd7 50. Qab5+ Ke7
51. Qbc3 Kf7 52. Rf1 Ke7 53. Qge3 Kf7 54. Qb8 Ke7 55. Qbc8 Kf7 56. Qhh8 Ke7 57. h3 e5 58. Qa8 Ke6 59. Qab8 Kf5 60. Qbc7 Ke6
61. Qhg7 Kf5 62. Qc3xe5# 1-0
)raw";
    ASSERT_EQ(expected_pgn, game.toPgn());
}

TEST_F(GameTestFixture, parsing_pgn_with_castle) {
    std::string const pgn = R"raw(
1. d4 d5 2. Nf3 e5 3. Bg5 Nf6 4. c4 dxc4 5. Nc3 Ne4 6. Qa4+ Bd7 7. h3 Nxc3 8. e3 Nd1 9. Bxc4 Nc3 10. g4 Nd5 11. O-O-O
    )raw";

    std::optional<Game> game_opt = Game::fromPgn(pgn);
    ASSERT_TRUE(game_opt.has_value());
    Game game = game_opt.value();

    ASSERT_EQ(21, game.movesSize());
    ASSERT_EQ("rn1qkb1r/pppb1ppp/8/3np1B1/Q1BP2P1/4PN1P/PP3P2/2KR3R b kq - 2 11", game.board().fen());

    ASSERT_EQ(Move({4,0}, {2, 0}), game.moveAt(21).value().move);
    ASSERT_EQ(white_king, game.moveAt(21).value().piece);
    ASSERT_FALSE(game.moveAt(21).value().isCheck);
    ASSERT_FALSE(game.moveAt(21).value().isCapture);
    ASSERT_FALSE(game.moveAt(21).value().isCheckmate);
    ASSERT_FALSE(game.moveAt(21).value().isSrcFileAmbigious);
    ASSERT_FALSE(game.moveAt(21).value().isSrcRankAmbigious);
    ASSERT_TRUE(game.moveAt(21).value().isCastle.has_value());
    ASSERT_EQ(Side::QueenSide, game.moveAt(21).value().isCastle.value());
    ASSERT_FALSE(game.result().has_value());

    std::string expected_pgn = R"raw([Event ""]
[Site ""]
[Date ""]
[Round ""]
[White ""]
[Black ""]
[Result "*"]

1. d4 d5 2. Nf3 e5 3. Bg5 Nf6 4. c4 dxc4 5. Nc3 Ne4 6. Qa4+ Bd7 7. h3 Nxc3 8. e3 Nd1 9. Bxc4 Nc3 10. g4 Nd5
11. O-O-O
)raw";
    ASSERT_EQ(expected_pgn, game.toPgn());
}

TEST_F(GameTestFixture, typical_game_flow) {
    Game game {};
    ASSERT_FALSE(game.makeMove(Move({3,1}, {3,4})));
    ASSERT_EQ(0, game.movesSize());
    ASSERT_TRUE(game.makeMove(Move({3,1}, {3,3})));
    ASSERT_EQ(1, game.movesSize());
    ASSERT_FALSE(game.makeMove(Move({3,1}, {3,3})));
    ASSERT_EQ(1, game.movesSize());

    ASSERT_TRUE(game.moveAt(1).has_value());
    ASSERT_EQ(Move({3,1}, {3,3}), game.moveAt(1).value().move);
    ASSERT_EQ(white_pawn, game.moveAt(1).value().piece);
    ASSERT_EQ(std::nullopt, game.moveAt(1).value().isCastle);
    ASSERT_FALSE(game.moveAt(1).value().isCheck);
    ASSERT_FALSE(game.moveAt(1).value().isCheckmate);
    ASSERT_FALSE(game.moveAt(1).value().isCapture);
    ASSERT_FALSE(game.moveAt(1).value().isSrcRankAmbigious);
    ASSERT_FALSE(game.moveAt(1).value().isSrcFileAmbigious);
    ASSERT_FALSE(game.result().has_value());
    ASSERT_EQ("rnbqkbnr/pppppppp/8/8/3P4/8/PPP1PPPP/RNBQKBNR b KQkq d3 0 1", game.board().fen());
    ASSERT_EQ("1. d4\n", game.toPgn(false));

    ASSERT_TRUE(game.makeMove(Move({4,6}, {4,4})));
    ASSERT_EQ(2, game.movesSize());
    ASSERT_TRUE(game.moveAt(2).has_value());
    ASSERT_EQ(Move({4,6}, {4,4}), game.moveAt(2).value().move);
    ASSERT_EQ(black_pawn, game.moveAt(2).value().piece);
    ASSERT_EQ(std::nullopt, game.moveAt(2).value().isCastle);
    ASSERT_FALSE(game.moveAt(2).value().isCheck);
    ASSERT_FALSE(game.moveAt(2).value().isCheckmate);
    ASSERT_FALSE(game.moveAt(2).value().isCapture);
    ASSERT_FALSE(game.moveAt(2).value().isSrcRankAmbigious);
    ASSERT_FALSE(game.moveAt(2).value().isSrcFileAmbigious);
    ASSERT_FALSE(game.result().has_value());
    ASSERT_EQ("rnbqkbnr/pppp1ppp/8/4p3/3P4/8/PPP1PPPP/RNBQKBNR w KQkq e6 0 2", game.board().fen());
    ASSERT_EQ("1. d4 e5\n", game.toPgn(false));

    ASSERT_TRUE(game.makeMove(Move({3,3}, {4,4})));
    ASSERT_EQ(3, game.movesSize());
    ASSERT_TRUE(game.moveAt(3).has_value());
    ASSERT_EQ(Move({3,3}, {4,4}), game.moveAt(3).value().move);
    ASSERT_EQ(white_pawn, game.moveAt(3).value().piece);
    ASSERT_EQ(std::nullopt, game.moveAt(3).value().isCastle);
    ASSERT_FALSE(game.moveAt(3).value().isCheck);
    ASSERT_FALSE(game.moveAt(3).value().isCheckmate);
    ASSERT_TRUE(game.moveAt(3).value().isCapture);
    ASSERT_FALSE(game.moveAt(3).value().isSrcRankAmbigious);
    ASSERT_TRUE(game.moveAt(3).value().isSrcFileAmbigious);
    ASSERT_FALSE(game.result().has_value());
    ASSERT_EQ("rnbqkbnr/pppp1ppp/8/4P3/8/8/PPP1PPPP/RNBQKBNR b KQkq - 0 2", game.board().fen());
    ASSERT_EQ("1. d4 e5 2. dxe5\n", game.toPgn(false));

    ASSERT_TRUE(game.makeMove(Move({3,6}, {3,5})));
    ASSERT_TRUE(game.makeMove(Move({2,0}, {5,3})));
    ASSERT_TRUE(game.makeMove(Move({6,7}, {5,5})));
    ASSERT_TRUE(game.makeMove(Move({4,4}, {5,5})));
    ASSERT_TRUE(game.makeMove(Move({3,7}, {5,5})));
    ASSERT_EQ(8, game.movesSize());

    ASSERT_TRUE(game.makeMove(Move({6,0}, {5,2})));
    ASSERT_TRUE(game.makeMove(Move({2,7}, {5,4})));
    ASSERT_TRUE(game.makeMove(Move({4,1}, {4,3})));
    ASSERT_TRUE(game.makeMove(Move({5,7}, {4,6})));
    ASSERT_TRUE(game.makeMove(Move({1,0}, {2,2})));
    ASSERT_TRUE(game.makeMove(Move({1,7}, {3,6})));
    ASSERT_TRUE(game.makeMove(Move({4,3}, {5,4})));
    ASSERT_TRUE(game.makeMove(Move({5,5}, {5,4})));
    ASSERT_EQ(16, game.movesSize());

    ASSERT_TRUE(game.makeMove(Move({5,0}, {3,2})));
    ASSERT_TRUE(game.makeMove(Move({3,6}, {4,4})));
    ASSERT_TRUE(game.makeMove(Move({3,2}, {5,4})));
    ASSERT_TRUE(game.makeMove(Move({3,5}, {3,4})));
    ASSERT_TRUE(game.makeMove(Move({5,2}, {4,4})));
    ASSERT_TRUE(game.makeMove(Move({2,6}, {2,4})));
    ASSERT_TRUE(game.makeMove(Move({2,2}, {3,4})));
    ASSERT_TRUE(game.makeMove(Move({0,7}, {3,7})));
    ASSERT_EQ(24, game.movesSize());

    ASSERT_TRUE(game.makeMove(Move({3,4}, {4,6})));
    ASSERT_TRUE(game.makeMove(Move({4,7}, {4,6})));
    ASSERT_TRUE(game.makeMove(Move({4,0}, {6,0})));

    ASSERT_EQ(Move({4,0}, {6,0}), game.moveAt(27).value().move);
    ASSERT_EQ(white_king, game.moveAt(27).value().piece);
    ASSERT_EQ(std::make_optional(Side::KingSide), game.moveAt(27).value().isCastle);
    ASSERT_FALSE(game.moveAt(27).value().isCheck);
    ASSERT_FALSE(game.moveAt(27).value().isCheckmate);
    ASSERT_FALSE(game.moveAt(27).value().isCapture);
    ASSERT_FALSE(game.moveAt(27).value().isSrcRankAmbigious);
    ASSERT_FALSE(game.moveAt(27).value().isSrcFileAmbigious);
    ASSERT_FALSE(game.result().has_value());
    ASSERT_EQ("3r3r/pp2kppp/8/2p1NB2/5B2/8/PPP2PPP/R2Q1RK1 b - - 1 14", game.board().fen());
    ASSERT_EQ(R"raw(1. d4 e5 2. dxe5 d6 3. Bf4 Nf6 4. exf6 Qxf6 5. Nf3 Bf5 6. e4 Be7 7. Nc3 Nd7 8. exf5 Qxf5 9. Bd3 Ne5 10. Bxf5 d5
11. Nxe5 c5 12. Nxd5 Rd8 13. Nxe7 Kxe7 14. O-O
)raw", game.toPgn(false));

    ASSERT_TRUE(game.makeMove(Move({1,6}, {1,4})));
    ASSERT_TRUE(game.makeMove(Move({4,4}, {2,5})));

    ASSERT_EQ(Move({4,4}, {2,5}), game.moveAt(29).value().move);
    ASSERT_EQ(white_knight, game.moveAt(29).value().piece);
    ASSERT_EQ(std::nullopt, game.moveAt(29).value().isCastle);
    ASSERT_TRUE(game.moveAt(29).value().isCheck);
    ASSERT_FALSE(game.moveAt(29).value().isCheckmate);
    ASSERT_FALSE(game.moveAt(29).value().isCapture);
    ASSERT_FALSE(game.moveAt(29).value().isSrcRankAmbigious);
    ASSERT_FALSE(game.moveAt(29).value().isSrcFileAmbigious);
    ASSERT_FALSE(game.result().has_value());
    ASSERT_EQ("3r3r/p3kppp/2N5/1pp2B2/5B2/8/PPP2PPP/R2Q1RK1 b - - 1 15", game.board().fen());
    ASSERT_EQ(R"raw(1. d4 e5 2. dxe5 d6 3. Bf4 Nf6 4. exf6 Qxf6 5. Nf3 Bf5 6. e4 Be7 7. Nc3 Nd7 8. exf5 Qxf5 9. Bd3 Ne5 10. Bxf5 d5
11. Nxe5 c5 12. Nxd5 Rd8 13. Nxe7 Kxe7 14. O-O b5 15. Nc6+
)raw", game.toPgn(false));

    ASSERT_TRUE(game.makeMove(Move({4,6}, {5,5})));
    ASSERT_TRUE(game.makeMove(Move({2,5}, {3,7})));
    ASSERT_TRUE(game.makeMove(Move({7,7}, {3,7})));
    ASSERT_EQ(32, game.movesSize());

    ASSERT_EQ(Move({7,7}, {3,7}), game.moveAt(32).value().move);
    ASSERT_EQ(black_rook, game.moveAt(32).value().piece);
    ASSERT_EQ(std::nullopt, game.moveAt(32).value().isCastle);
    ASSERT_FALSE(game.moveAt(32).value().isCheck);
    ASSERT_FALSE(game.moveAt(32).value().isCheckmate);
    ASSERT_TRUE(game.moveAt(32).value().isCapture);
    ASSERT_FALSE(game.moveAt(32).value().isSrcRankAmbigious);
    ASSERT_FALSE(game.moveAt(32).value().isSrcFileAmbigious);
    ASSERT_FALSE(game.result().has_value());
    ASSERT_EQ("3r4/p4ppp/5k2/1pp2B2/5B2/8/PPP2PPP/R2Q1RK1 w - - 0 17", game.board().fen());
    ASSERT_EQ(R"raw(1. d4 e5 2. dxe5 d6 3. Bf4 Nf6 4. exf6 Qxf6 5. Nf3 Bf5 6. e4 Be7 7. Nc3 Nd7 8. exf5 Qxf5 9. Bd3 Ne5 10. Bxf5 d5
11. Nxe5 c5 12. Nxd5 Rd8 13. Nxe7 Kxe7 14. O-O b5 15. Nc6+ Kf6 16. Nxd8 Rxd8
)raw", game.toPgn(false));

    ASSERT_TRUE(game.makeMove(Move({3,0}, {3,7})));
    ASSERT_TRUE(game.makeMove(Move({5,5}, {5,4})));
    ASSERT_TRUE(game.makeMove(Move({3,7}, {7,7})));
    ASSERT_TRUE(game.makeMove(Move({5,4}, {5,3})));
    ASSERT_TRUE(game.makeMove(Move({7,7}, {7,6})));
    ASSERT_TRUE(game.makeMove(Move({5,3}, {4,4})));
    ASSERT_TRUE(game.makeMove(Move({7,6}, {6,6})));
    ASSERT_TRUE(game.makeMove(Move({4,4}, {4,5})));
    ASSERT_EQ(40, game.movesSize());

    ASSERT_TRUE(game.makeMove(Move({6,6}, {5,7})));
    ASSERT_TRUE(game.makeMove(Move({4,5}, {5,5})));
    ASSERT_TRUE(game.makeMove(Move({5,7}, {0,7})));
    ASSERT_TRUE(game.makeMove(Move({2,4}, {2,3})));
    ASSERT_TRUE(game.makeMove(Move({5,0}, {4,0})));

    ASSERT_EQ(Move({5, 0}, {4,0}), game.moveAt(45).value().move);
    ASSERT_EQ(white_rook, game.moveAt(45).value().piece);
    ASSERT_EQ(std::nullopt, game.moveAt(45).value().isCastle);
    ASSERT_FALSE(game.moveAt(45).value().isCheck);
    ASSERT_FALSE(game.moveAt(45).value().isCheckmate);
    ASSERT_FALSE(game.moveAt(45).value().isCapture);
    ASSERT_FALSE(game.moveAt(45).value().isSrcRankAmbigious);
    ASSERT_TRUE(game.moveAt(45).value().isSrcFileAmbigious);
    ASSERT_FALSE(game.result().has_value());
    ASSERT_EQ("Q7/p4p2/5k2/1p6/2p5/8/PPP2PPP/R3R1K1 b - - 1 23", game.board().fen());
    ASSERT_EQ(R"raw(1. d4 e5 2. dxe5 d6 3. Bf4 Nf6 4. exf6 Qxf6 5. Nf3 Bf5 6. e4 Be7 7. Nc3 Nd7 8. exf5 Qxf5 9. Bd3 Ne5 10. Bxf5 d5
11. Nxe5 c5 12. Nxd5 Rd8 13. Nxe7 Kxe7 14. O-O b5 15. Nc6+ Kf6 16. Nxd8 Rxd8 17. Qxd8+ Kxf5 18. Qh8 Kxf4 19. Qxh7 Ke5 20. Qxg7+ Ke6
21. Qf8 Kf6 22. Qa8 c4 23. Rfe1
)raw", game.toPgn(false));

    ASSERT_TRUE(game.makeMove(Move({5,5}, {6,6})));
    ASSERT_TRUE(game.makeMove(Move({0,7}, {0,6})));
    ASSERT_TRUE(game.makeMove(Move({6,6}, {5,5})));
    ASSERT_EQ(48, game.movesSize());

    ASSERT_TRUE(game.makeMove(Move({0,6}, {2,4})));
    ASSERT_TRUE(game.makeMove(Move({5,5}, {6,6})));
    ASSERT_TRUE(game.makeMove(Move({2,4}, {1,4})));
    ASSERT_TRUE(game.makeMove(Move({6,6}, {5,5})));
    ASSERT_TRUE(game.makeMove(Move({1,4}, {2,3})));
    ASSERT_TRUE(game.makeMove(Move({5,5}, {6,6})));
    ASSERT_TRUE(game.makeMove(Move({0,1}, {0,3})));
    ASSERT_TRUE(game.makeMove(Move({6,6}, {5,5})));
    ASSERT_EQ(56, game.movesSize());

    ASSERT_TRUE(game.makeMove(Move({0,3}, {0,4})));
    ASSERT_TRUE(game.makeMove(Move({5,5}, {6,6})));
    ASSERT_TRUE(game.makeMove(Move({0,4}, {0,5})));
    ASSERT_TRUE(game.makeMove(Move({6,6}, {5,5})));
    ASSERT_TRUE(game.makeMove(Move({0,5}, {0,6})));
    ASSERT_TRUE(game.makeMove(Move({5,5}, {6,6})));
    ASSERT_TRUE(game.makeMove(Move({1,1}, {1,3})));
    ASSERT_TRUE(game.makeMove(Move({6,6}, {5,5})));
    ASSERT_EQ(64, game.movesSize());

    ASSERT_TRUE(game.makeMove(Move({1,3}, {1,4})));
    ASSERT_TRUE(game.makeMove(Move({5,5}, {6,6})));
    ASSERT_TRUE(game.makeMove(Move({1,4}, {1,5})));
    ASSERT_TRUE(game.makeMove(Move({6,6}, {5,5})));
    ASSERT_TRUE(game.makeMove(Move({2,3}, {1,2})));
    ASSERT_TRUE(game.makeMove(Move({5,5}, {6,6})));
    ASSERT_TRUE(game.makeMove(Move({2,1}, {2,3})));
    ASSERT_TRUE(game.makeMove(Move({6,6}, {5,5})));
    ASSERT_EQ(72, game.movesSize());

    ASSERT_TRUE(game.makeMove(Move({1,5}, {1,6})));
    ASSERT_TRUE(game.makeMove(Move({5,5}, {6,6})));
    ASSERT_TRUE(game.makeMove(Move({2,3}, {2,4})));
    ASSERT_TRUE(game.makeMove(Move({6,6}, {5,5})));
    ASSERT_TRUE(game.makeMove(Move({4,0}, {4,5})));
    ASSERT_TRUE(game.makeMove(Move({5,6}, {4,5})));
    ASSERT_TRUE(game.makeMove(Move({0,0}, {4,0})));
    ASSERT_TRUE(game.makeMove(Move({5,5}, {4,6})));
    ASSERT_EQ(80, game.movesSize());
    ASSERT_EQ("8/PP2k3/4p3/2P5/8/1Q6/5PPP/4R1K1 w - - 2 41", game.board().fen());

    ASSERT_TRUE(game.makeMove(Move({1,6}, {1,7}, Piece::Type::Queen)));
    ASSERT_TRUE(game.makeMove(Move({4,6}, {5,5})));
    ASSERT_TRUE(game.makeMove(Move({0,6}, {0,7}, Piece::Type::Queen)));
    ASSERT_TRUE(game.makeMove(Move({5,5}, {4,6})));
    ASSERT_TRUE(game.makeMove(Move({2,4}, {2,5})));
    ASSERT_TRUE(game.makeMove(Move({4,6}, {5,5})));
    ASSERT_TRUE(game.makeMove(Move({2,5}, {2,6})));
    ASSERT_TRUE(game.makeMove(Move({5,5}, {4,6})));
    ASSERT_EQ(88, game.movesSize());

    ASSERT_TRUE(game.makeMove(Move({2,6}, {2,7}, Piece::Type::Queen)));

    ASSERT_EQ(Move({2,6}, {2,7}, Piece::Type::Queen), game.moveAt(89).value().move);
    ASSERT_EQ(white_pawn, game.moveAt(89).value().piece);
    ASSERT_EQ(std::nullopt, game.moveAt(89).value().isCastle);
    ASSERT_FALSE(game.moveAt(89).value().isCheck);
    ASSERT_FALSE(game.moveAt(89).value().isCheckmate);
    ASSERT_FALSE(game.moveAt(89).value().isCapture);
    ASSERT_FALSE(game.moveAt(89).value().isSrcRankAmbigious);
    ASSERT_FALSE(game.moveAt(89).value().isSrcFileAmbigious);
    ASSERT_FALSE(game.result().has_value());
    ASSERT_EQ("QQQ5/4k3/4p3/8/8/1Q6/5PPP/4R1K1 b - - 0 45", game.board().fen());
    ASSERT_EQ(R"raw(1. d4 e5 2. dxe5 d6 3. Bf4 Nf6 4. exf6 Qxf6 5. Nf3 Bf5 6. e4 Be7 7. Nc3 Nd7 8. exf5 Qxf5 9. Bd3 Ne5 10. Bxf5 d5
11. Nxe5 c5 12. Nxd5 Rd8 13. Nxe7 Kxe7 14. O-O b5 15. Nc6+ Kf6 16. Nxd8 Rxd8 17. Qxd8+ Kxf5 18. Qh8 Kxf4 19. Qxh7 Ke5 20. Qxg7+ Ke6
21. Qf8 Kf6 22. Qa8 c4 23. Rfe1 Kg7 24. Qxa7 Kf6 25. Qc5 Kg7 26. Qxb5 Kf6 27. Qxc4 Kg7 28. a4 Kf6 29. a5 Kg7 30. a6 Kf6
31. a7 Kg7 32. b4 Kf6 33. b5 Kg7 34. b6 Kf6 35. Qb3 Kg7 36. c4 Kf6 37. b7 Kg7 38. c5 Kf6 39. Re6+ fxe6 40. Re1 Ke7
41. b8=Q Kf6 42. a8=Q Ke7 43. c6 Kf6 44. c7 Ke7 45. c8=Q
)raw", game.toPgn(false));

    ASSERT_TRUE(game.makeMove(Move({4,6}, {5,5})));
    ASSERT_TRUE(game.makeMove(Move({1,7}, {6,2})));
    ASSERT_TRUE(game.makeMove(Move({5,5}, {4,6})));
    ASSERT_TRUE(game.makeMove(Move({2,7}, {0,5})));
    ASSERT_TRUE(game.makeMove(Move({4,6}, {3,6})));
    ASSERT_TRUE(game.makeMove(Move({0,7}, {7,7})));
    ASSERT_TRUE(game.makeMove(Move({3,6}, {4,6})));
    ASSERT_EQ(96, game.movesSize());

    ASSERT_TRUE(game.makeMove(Move({7,7}, {7,5})));
    ASSERT_TRUE(game.makeMove(Move({4,6}, {3,6})));
    ASSERT_TRUE(game.makeMove(Move({0,5}, {1,4})));
    ASSERT_TRUE(game.makeMove(Move({3,6}, {4,6})));
    ASSERT_TRUE(game.makeMove(Move({1,2}, {2,2})));
    ASSERT_TRUE(game.makeMove(Move({4,6}, {5,6})));
    ASSERT_TRUE(game.makeMove(Move({4,0}, {5,0})));
    ASSERT_TRUE(game.makeMove(Move({5,6}, {4,6})));
    ASSERT_EQ(104, game.movesSize());

    ASSERT_TRUE(game.makeMove(Move({6,2}, {4,2})));
    ASSERT_TRUE(game.makeMove(Move({4,6}, {5,6})));
    ASSERT_TRUE(game.makeMove(Move({1,4}, {1,7})));
    ASSERT_TRUE(game.makeMove(Move({5,6}, {4,6})));
    ASSERT_TRUE(game.makeMove(Move({1,7}, {2,7})));
    ASSERT_TRUE(game.makeMove(Move({4,6}, {5,6})));
    ASSERT_TRUE(game.makeMove(Move({7,5}, {7,7})));
    ASSERT_TRUE(game.makeMove(Move({5,6}, {4,6})));
    ASSERT_EQ(112, game.movesSize());

    ASSERT_TRUE(game.makeMove(Move({7,1}, {7,2})));
    ASSERT_TRUE(game.makeMove(Move({4,5}, {4,4})));
    ASSERT_TRUE(game.makeMove(Move({2,7}, {0,7})));
    ASSERT_TRUE(game.makeMove(Move({4,6}, {4,5})));
    ASSERT_TRUE(game.makeMove(Move({0,7}, {1,7})));
    ASSERT_TRUE(game.makeMove(Move({4,5}, {5,4})));
    ASSERT_TRUE(game.makeMove(Move({1,7}, {2,6})));
    ASSERT_TRUE(game.makeMove(Move({5,4}, {4,5})));
    ASSERT_EQ(120, game.movesSize());

    ASSERT_TRUE(game.makeMove(Move({7,7}, {6,6})));
    ASSERT_TRUE(game.makeMove(Move({4,5}, {5,4})));
    ASSERT_TRUE(game.makeMove(Move({2,2}, {4,4})));
    ASSERT_EQ(123, game.movesSize());

    ASSERT_EQ(Move({2,2}, {4,4}), game.moveAt(123).value().move);
    ASSERT_EQ(white_queen, game.moveAt(123).value().piece);
    ASSERT_EQ(std::nullopt, game.moveAt(123).value().isCastle);
    ASSERT_FALSE(game.moveAt(123).value().isCheck);
    ASSERT_TRUE(game.moveAt(123).value().isCheckmate);
    ASSERT_TRUE(game.moveAt(123).value().isCapture);
    ASSERT_TRUE(game.moveAt(123).value().isSrcRankAmbigious);
    ASSERT_TRUE(game.moveAt(123).value().isSrcFileAmbigious);
    ASSERT_TRUE(game.result().has_value());
    ASSERT_EQ("8/2Q3Q1/8/4Qk2/8/4Q2P/5PP1/5RK1 b - - 0 62", game.board().fen());
    ASSERT_EQ(R"raw(1. d4 e5 2. dxe5 d6 3. Bf4 Nf6 4. exf6 Qxf6 5. Nf3 Bf5 6. e4 Be7 7. Nc3 Nd7 8. exf5 Qxf5 9. Bd3 Ne5 10. Bxf5 d5
11. Nxe5 c5 12. Nxd5 Rd8 13. Nxe7 Kxe7 14. O-O b5 15. Nc6+ Kf6 16. Nxd8 Rxd8 17. Qxd8+ Kxf5 18. Qh8 Kxf4 19. Qxh7 Ke5 20. Qxg7+ Ke6
21. Qf8 Kf6 22. Qa8 c4 23. Rfe1 Kg7 24. Qxa7 Kf6 25. Qc5 Kg7 26. Qxb5 Kf6 27. Qxc4 Kg7 28. a4 Kf6 29. a5 Kg7 30. a6 Kf6
31. a7 Kg7 32. b4 Kf6 33. b5 Kg7 34. b6 Kf6 35. Qb3 Kg7 36. c4 Kf6 37. b7 Kg7 38. c5 Kf6 39. Re6+ fxe6 40. Re1 Ke7
41. b8=Q Kf6 42. a8=Q Ke7 43. c6 Kf6 44. c7 Ke7 45. c8=Q Kf6 46. Q8g3 Ke7 47. Qca6 Kd7 48. Qh8 Ke7 49. Qh6 Kd7 50. Qab5+ Ke7
51. Qbc3 Kf7 52. Rf1 Ke7 53. Qge3 Kf7 54. Qb8 Ke7 55. Qbc8 Kf7 56. Qhh8 Ke7 57. h3 e5 58. Qa8 Ke6 59. Qab8 Kf5 60. Qbc7 Ke6
61. Qhg7 Kf5 62. Qc3xe5# 1-0
)raw", game.toPgn(false));

    // game is already over so should not be able to make move
    ASSERT_FALSE(game.makeMove(Move({5,4}, {6,5})));
}

TEST_F(GameTestFixture, game_is_aware_of_threefold_repetition) {
    auto game_opt = Game::fromPgn(R"raw(
1. e4 e5 2. Ke2 Ke7 3. Ke1 Ke8 4. Nf3 Nf6 5. Ng1 Ng8
)raw");
    Game game = game_opt.value();
    ASSERT_EQ("rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w - - 8 6", game.board().fen());
    ASSERT_EQ(10, game.movesSize());

    ASSERT_TRUE(game.makeMove(Move({5,0}, {2,3})));
    ASSERT_TRUE(game.makeMove(Move({5,7}, {2,4})));
    ASSERT_TRUE(game.makeMove(Move({2,3}, {5,0})));
    ASSERT_EQ(std::nullopt, game.result());
    ASSERT_TRUE(game.makeMove(Move({2,4}, {5,7})));

    ASSERT_EQ(std::make_optional(ResultType::Draw), game.result());
    ASSERT_EQ("rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w - - 12 8", game.board().fen());
    ASSERT_FALSE(game.makeMove(Move({0,1}, {0,2})));
    ASSERT_FALSE(game.makeMove(Move({0,6}, {0,5})));
    ASSERT_EQ(14, game.movesSize());
}

TEST_F(GameTestFixture, game_is_aware_of_fifty_move_rule) {
    auto game_opt = Game::fromPgn(R"raw(
[Event "Rated Bullet game"]
[Site "https://lichess.org/WPURzP0g"]
[Date "2023.04.19"]
1. Nf3 Nf6 2. g3 d6 3. Bg2 g6 4. d3 Bg7 5. O-O O-O 6. e4 Bd7 7. Nc3 Qc8 8. Re1 Nc6 9. Bh1 e5 10. Be3 Nd4
11. Nxd4 exd4 12. Bxd4 Re8 13. Bxf6 Bxf6 14. Nd5 Bg7 15. Rb1 c6 16. Ne3 a5 17. Qd2 Qc7 18. c3 b5 19. d4 a4 20. a3 h5
21. Rbd1 Rac8 22. f4 Rcd8 23. e5 Bc8 24. Qf2 d5 25. Bf3 Qe7 26. Be2 Bh3 27. f5 Qg5 28. Bd3 Bh6 29. Ng2 Bxg2 30. Kxg2 c5
31. dxc5 h4 32. Rf1 Rxe5 33. fxg6 f6 34. Qxf6 Qxf6 35. Rxf6 hxg3 36. hxg3 d4 37. cxd4 Rxd4 38. Bc2 Rxd1 39. Bxd1 Rxc5 40. Rf2 Rc1
41. Be2 Rc5 42. Bf3 Kg7 43. Re2 Bg5 44. Be4 Bf6 45. Bf3 Rg5 46. b3 Be5 47. bxa4 bxa4 48. Kf2 Bxg3+ 49. Ke3 Bd6 50. Kd3 Bc5
51. Bd5 Rxd5+ 52. Kc4 Rg5 53. Kb5 Bxa3+ 54. Kxa4 Bd6 55. Kb3 Be5 56. Kc4 Bf6 57. Kd3 Rg3+ 58. Ke4 Rg4+ 59. Kf3 Rg5 60. Ke3 Re5+
61. Kd3 Rc5 62. Kd2 Rc3 63. Kd1 Rc7 64. Rc2 Re7 65. Kc1 Re1+ 66. Kd2 Re4 67. Kd3 Rb4 68. Rd2 Rb3+ 69. Ke2 Rb5 70. Kd1 Re5
71. Rd3 Rg5 72. Rd2 Kxg6 73. Re2 Bd4 74. Kc2 Be3 75. Kd3 Bf4 76. Ke4 Re5+ 77. Kf3 Rf5 78. Ke4 Bg5 79. Kd3 Rd5+ 80. Kc2 Rb5
81. Kd1 Rb6 82. Rf2 Rf6 83. Ke2 Re6+ 84. Kf1 Rc6 85. Rg2 Kh5 86. Rf2 Bf4 87. Ke2 Kg4 88. Kf1 Kf5 89. Re2 Rc1+ 90. Kf2 Rc3
91. Rb2 Kg4 92. Rb3
)raw");
    Game game = game_opt.value();
    ASSERT_EQ("8/8/8/8/5bk1/1Rr5/5K2/8 b - - 39 92", game.board().fen());
    ASSERT_EQ(183, game.movesSize());
    ASSERT_TRUE(game.makeMove(Move({2,2}, {2,0})));
    ASSERT_TRUE(game.makeMove(Move({1,2}, {1,1})));
    ASSERT_TRUE(game.makeMove(Move({2,0}, {2,7})));
    ASSERT_TRUE(game.makeMove(Move({1,1}, {1,7})));
    ASSERT_TRUE(game.makeMove(Move({2,7}, {2,1})));
    ASSERT_TRUE(game.makeMove(Move({5,1}, {5,0})));
    ASSERT_TRUE(game.makeMove(Move({2,1}, {2,2})));
    ASSERT_TRUE(game.makeMove(Move({1,7}, {1,1})));
    ASSERT_EQ(191, game.movesSize());
    ASSERT_EQ("8/8/8/8/5bk1/2r5/1R6/5K2 b - - 47 96", game.board().fen());

    ASSERT_TRUE(game.makeMove(Move({2,2}, {0,2})));
    ASSERT_TRUE(game.makeMove(Move({1,1}, {0,1})));
    ASSERT_EQ(193, game.movesSize());
    ASSERT_EQ("8/8/8/8/5bk1/r7/R7/5K2 b - - 49 97", game.board().fen());
    ASSERT_EQ(std::nullopt, game.result());

    ASSERT_TRUE(game.makeMove(Move({0,2}, {0,6})));
    ASSERT_EQ(194, game.movesSize());
    ASSERT_EQ(std::make_optional(ResultType::Draw), game.result());
    ASSERT_EQ("8/r7/8/8/5bk1/8/R7/5K2 w - - 50 98", game.board().fen());

    ASSERT_FALSE(game.makeMove(Move({0,1}, {0,0})));
}
