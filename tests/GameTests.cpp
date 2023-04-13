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

    const std::string starting_position_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
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


    ASSERT_EQ(game.moveAt(1).value(), game.moveAt(1, Color::White));
    ASSERT_EQ(game.moveAt(2).value(), game.moveAt(1, Color::Black));
    ASSERT_EQ(game.moveAt(5).value(), game.moveAt(3, Color::White));
    ASSERT_EQ(game.moveAt(10).value(), game.moveAt(5, Color::Black));

    ASSERT_EQ(Move({white_pawn, {4,1}, {4, 3}}), game.moveAt(1).value().move);
    ASSERT_EQ(white_pawn, game.moveAt(1).value().piece);
    ASSERT_FALSE(game.moveAt(1).value().isCheck);
    ASSERT_FALSE(game.moveAt(1).value().isCapture);
    ASSERT_FALSE(game.moveAt(1).value().isCheckmate);
    ASSERT_FALSE(game.moveAt(1).value().isSrcFileAmbigious);
    ASSERT_FALSE(game.moveAt(1).value().isSrcRankAmbigious);
    ASSERT_FALSE(game.moveAt(1).value().isCastle.has_value());

    ASSERT_EQ(Move({black_pawn, {4,6}, {4, 4}}), game.moveAt(2).value().move);
    ASSERT_EQ(black_pawn, game.moveAt(2).value().piece);
    ASSERT_FALSE(game.moveAt(2).value().isCheck);
    ASSERT_FALSE(game.moveAt(2).value().isCapture);
    ASSERT_FALSE(game.moveAt(2).value().isCheckmate);
    ASSERT_FALSE(game.moveAt(2).value().isSrcFileAmbigious);
    ASSERT_FALSE(game.moveAt(2).value().isSrcRankAmbigious);
    ASSERT_FALSE(game.moveAt(2).value().isCastle.has_value());

    ASSERT_EQ(Move({white_king, {4,0}, {6, 0}}), game.moveAt(9).value().move);
    ASSERT_EQ(white_king, game.moveAt(9).value().piece);
    ASSERT_FALSE(game.moveAt(9).value().isCheck);
    ASSERT_FALSE(game.moveAt(9).value().isCapture);
    ASSERT_FALSE(game.moveAt(9).value().isCheckmate);
    ASSERT_FALSE(game.moveAt(9).value().isSrcFileAmbigious);
    ASSERT_FALSE(game.moveAt(9).value().isSrcRankAmbigious);
    ASSERT_EQ(Side::KingSide, game.moveAt(9).value().isCastle.value());

    ASSERT_EQ(Move({black_knight, {1,7}, {3, 6}}), game.moveAt(20).value().move);
    ASSERT_EQ(black_knight, game.moveAt(20).value().piece);
    ASSERT_FALSE(game.moveAt(20).value().isCheck);
    ASSERT_FALSE(game.moveAt(20).value().isCapture);
    ASSERT_FALSE(game.moveAt(20).value().isCheckmate);
    ASSERT_TRUE(game.moveAt(20).value().isSrcFileAmbigious);
    ASSERT_FALSE(game.moveAt(20).value().isSrcRankAmbigious);
    ASSERT_FALSE(game.moveAt(20).value().isCastle.has_value());

    ASSERT_EQ(Move({white_bishop, {1,2}, {2, 3}}), game.moveAt(43).value().move);
    ASSERT_EQ(white_bishop, game.moveAt(43).value().piece);
    ASSERT_FALSE(game.moveAt(43).value().isCheck);
    ASSERT_TRUE(game.moveAt(43).value().isCapture);
    ASSERT_FALSE(game.moveAt(43).value().isCheckmate);
    ASSERT_FALSE(game.moveAt(43).value().isSrcFileAmbigious);
    ASSERT_FALSE(game.moveAt(43).value().isSrcRankAmbigious);
    ASSERT_FALSE(game.moveAt(43).value().isCastle.has_value());

    ASSERT_EQ(Move({black_rook, {4,7}, {4, 0}}), game.moveAt(50).value().move);
    ASSERT_EQ(black_rook, game.moveAt(50).value().piece);
    ASSERT_TRUE(game.moveAt(50).value().isCheck);
    ASSERT_TRUE(game.moveAt(50).value().isCapture);
    ASSERT_FALSE(game.moveAt(50).value().isCheckmate);
    ASSERT_FALSE(game.moveAt(50).value().isSrcFileAmbigious);
    ASSERT_FALSE(game.moveAt(50).value().isSrcRankAmbigious);
    ASSERT_FALSE(game.moveAt(50).value().isCastle.has_value());
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

1. d4 Nf6 2. c4 d6 3. Nf3 g6 4. Nc3 Bg7 5. Bg5 { E61 King's Indian Defense: Smyslov Variation } h6 6. Bh4 g5 7. Bg3 Nh5 8. e3 c5 9. d5 Bxc3+ 10. bxc3 Kd7 11. Ne5+ dxe5 12. Bxe5 Qa5 13. Bxh8 f6 14. Rc1 Qd8 15. Qxh5 Qxh8 16. Be2 Kc7 17. h4 Qg7 18. hxg5 hxg5 19. Qh7 Qxh7 20. Rxh7 Kd6 21. Rh8 Nd7 22. Bg4 b6 23. Kd2 Rb8 24. a4 Ba6 25. Rxb8 Nxb8 26. Kd3 e5 27. Rh1 e4+ 28. Kxe4 Bxc4 29. Be6 Bb3 30. Kf5 Bxa4 31. Kxf6 Bc2 32. Rh7 Bxh7 33. f3 a5 34. e4 Nd7+ 35. Bxd7 Kxd7 36. e5 Bg8 37. e6+ Kd6 38. e7 Kd7 39. d6 a4 40. Kg7 Bh7 41. Kf7 Bg6+ 42. Kxg6 a3 43. Kf7 a2 44. e8=Q+ Kxd6 45. Qe6+ Kc7 46. Qxa2 Kc6 47. Qe6+ Kb5 48. Qd5 Ka5 49. Qc4 b5 50. Qxc5 Ka4 51. Qb4# { White wins by checkmate. } 1-0
    )raw";

    std::optional<Game> game_opt = Game::fromPgn(pgn);
    ASSERT_TRUE(game_opt.has_value());
    Game game = game_opt.value();

    ASSERT_EQ(Move({white_pawn, {4,6}, {4, 7}, std::make_optional(Piece::Type::Queen)}), game.moveAt(87).value().move);
    ASSERT_EQ(white_pawn, game.moveAt(87).value().piece);
    ASSERT_TRUE(game.moveAt(87).value().isCheck);
    ASSERT_FALSE(game.moveAt(87).value().isCapture);
    ASSERT_FALSE(game.moveAt(87).value().isCheckmate);
    ASSERT_FALSE(game.moveAt(87).value().isSrcFileAmbigious);
    ASSERT_FALSE(game.moveAt(87).value().isSrcRankAmbigious);
    ASSERT_FALSE(game.moveAt(87).value().isCastle.has_value());

    ASSERT_EQ(Move({white_queen, {2,4}, {1, 3}}), game.moveAt(101).value().move);
    ASSERT_EQ(white_queen, game.moveAt(101).value().piece);
    ASSERT_TRUE(game.moveAt(101).value().isCheck);
    ASSERT_FALSE(game.moveAt(101).value().isCapture);
    ASSERT_TRUE(game.moveAt(101).value().isCheckmate);
    ASSERT_FALSE(game.moveAt(101).value().isSrcFileAmbigious);
    ASSERT_FALSE(game.moveAt(101).value().isSrcRankAmbigious);
    ASSERT_FALSE(game.moveAt(101).value().isCastle.has_value());
}

TEST_F(GameTestFixture, parsing_pgn_with_ambigious_rank) {
    std::string const pgn = R"raw(
1. d4 d5 2. Nf3 Nf6 3. Nc3 Na6 4. Bd2 Bd7 5. Na4 Rb8 6. Ne5 Qc8 7. Nc5 g6 8. Ncd3 Ng8 9. f3 Kd8 10. Nf2 Ke8 11. Nc4 e6 12. Nd3 Ra8 13. Na5 Be7 14. Nc1 f6 15. a4 g5 16. Na2 Rb8 17. Nb4 Ra8 18. Nxd5 Nh6 19. Ne3 Rb8 20. c3 Ra8 21. Nc2 Rb8 22. Na3 Kf7 23. N3c4
    )raw";

    std::optional<Game> game_opt = Game::fromPgn(pgn);
    ASSERT_TRUE(game_opt.has_value());
    Game game = game_opt.value();

    ASSERT_EQ(Move(white_knight, {0,2}, {2, 3}), game.moveAt(45).value().move);
    ASSERT_EQ(white_knight, game.moveAt(45).value().piece);
    ASSERT_FALSE(game.moveAt(45).value().isCheck);
    ASSERT_FALSE(game.moveAt(45).value().isCapture);
    ASSERT_FALSE(game.moveAt(45).value().isCheckmate);
    ASSERT_FALSE(game.moveAt(45).value().isSrcFileAmbigious);
    ASSERT_TRUE(game.moveAt(45).value().isSrcRankAmbigious);
}

TEST_F(GameTestFixture, parsing_pgn_with_ambigious_rank_and_file_both) {
    std::string const pgn = R"raw(
    )raw";

    ASSERT_TRUE(false); // todo

    std::optional<Game> game_opt = Game::fromPgn(pgn);
    ASSERT_TRUE(game_opt.has_value());
    Game game = game_opt.value();

    ASSERT_EQ(Move(white_knight, {0,2}, {2, 3}), game.moveAt(45).value().move);
    ASSERT_EQ(white_knight, game.moveAt(45).value().piece);
    ASSERT_FALSE(game.moveAt(45).value().isCheck);
    ASSERT_FALSE(game.moveAt(45).value().isCapture);
    ASSERT_FALSE(game.moveAt(45).value().isCheckmate);
    ASSERT_FALSE(game.moveAt(45).value().isSrcFileAmbigious);
    ASSERT_TRUE(game.moveAt(45).value().isSrcRankAmbigious);
}
