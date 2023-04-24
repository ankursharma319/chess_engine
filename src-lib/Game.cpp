#include "Game.hpp"
#include "Board.hpp"
#include "GameEngine.hpp"
#include "Move.hpp"
#include "glog/logging.h"

#include <cassert>
#include <cctype>
#include <exception>
#include <optional>
#include <stdexcept>

namespace {

using namespace ChessEngineLib;

bool is_spacy(char c) {
    return c == ' ' || c == '\n' || c == '\t';
}

bool is_non_spacy(char c) {
    return !is_spacy(c);
}

std::optional<std::pair<Game::SevenTagRoster, std::size_t>> parseRoster(std::string const& pgn) {
    std::size_t i = 0;
    Game::SevenTagRoster roster {};
    enum class ParseMode {
        None, Key, ExpectingValue, Value
    };
    ParseMode current_mode = ParseMode::None;
    std::string current_key {};
    std::string current_value {};
    while (i < pgn.size()) {
        char c = pgn.at(i);
        if (current_mode == ParseMode::None) {
            if (c == '[') {
                current_mode = ParseMode::Key;
            } else if (c == '1') {
                return std::make_optional<std::pair<Game::SevenTagRoster, std::size_t>>(roster, i);
            } else if (is_non_spacy(c)) {
                VLOG(2) << "is_non_spacy in None mode, return nullopt";
                return std::nullopt;
            }
        } else if (current_mode == ParseMode::Key) {
            if (c == ' ') {
                current_mode = ParseMode::ExpectingValue;
                VLOG(2) << "Parsed key " << current_key;
            } else {
                current_key.push_back(c);
            }
        } else if (current_mode == ParseMode::ExpectingValue) {
            if (c == '"') {
                current_mode = ParseMode::Value;
            } else {
                return std::nullopt;
            }
        } else if (current_mode == ParseMode::Value) {
            if (c == '"') {
                VLOG(2) << "Parsed key " << current_key << ", value=" << current_value;
                i++;
                if (pgn.size() <= i || pgn.at(i) != ']') {
                    VLOG(2) << "Didnt receive the ending roster tag";
                    return std::nullopt;
                }
                if (current_key == "Event") {
                    roster.event = current_value;
                } else if (current_key == "Date") {
                    roster.date = current_value;
                } else if (current_key == "Site") {
                    roster.site = current_value;
                } else if (current_key == "Round") {
                    roster.round = current_value;
                } else if (current_key == "Black") {
                    roster.black = current_value;
                } else if (current_key == "White") {
                    roster.white = current_value;
                } else if (current_key == "Result") {
                    if (current_value == "1/2-1/2") {
                        roster.result = ResultType::Draw;
                    } else if (current_value == "1-0") {
                        roster.result = ResultType::WhiteWin;
                    } else if (current_value == "0-1") {
                        roster.result = ResultType::BlackWin;
                    }
                }
                current_key = "";
                current_value = "";
                current_mode = ParseMode::None;
            } else {
                current_value.push_back(c);
            }
        }
        i++;
    }
    return std::make_optional<std::pair<Game::SevenTagRoster, std::size_t>>(roster, i);
}

Piece::Type from_pgn_to_piece_type(char c) {
    switch(c) {
        case 'K':
            return Piece::Type::King;
        case 'Q':
            return Piece::Type::Queen;
        case 'B':
            return Piece::Type::Bishop;
        case 'R':
            return Piece::Type::Rook;
        case 'N':
            return Piece::Type::Knight;
        default:
            return Piece::Type::Pawn;
    }
}

std::optional<Square> find_src_square(
    Board const& board, Piece const& piece, Square to,
    std::optional<Piece::Type> const& promotionTo
) {
    for (std::uint8_t col=0; col < 8; col++) {
        for (std::uint8_t row=0; row < 8; row++) {
            if (
                auto const& opt = board.at(Square {col, row});
                opt.has_value() && opt.value() == piece
            ) {
                Move mv = {{col, row}, to, promotionTo};
                if (isMoveLegal(board, std::move(mv))) {
                    return Square {col, row};
                }
            }
        }
    }
    return std::nullopt;
}

std::optional<std::uint8_t> find_src_row(
    Board const& board, Piece const& piece, Square to, std::uint8_t from_col,
    std::optional<Piece::Type> const& promotionTo
) {
    VLOG(7) << "finding src row for " << piece << " to " << to;
    for (std::uint8_t row=0; row < 8; row++) {
        if (
            auto const& opt = board.at(Square {from_col, row});
            opt.has_value() && opt.value() == piece
        ) {
            Move mv = {{from_col, row}, to, promotionTo};
            if (isMoveLegal(board, std::move(mv))) {
                return row;
            }
        }
    }
    return std::nullopt;
}

std::optional<std::uint8_t> find_src_col(
    Board const& board, Piece const& piece, Square to, std::uint8_t from_row,
    std::optional<Piece::Type> const& promotionTo
) {
    VLOG(7) << "finding src column for " << piece << " to " << to;
    for (std::uint8_t col=0; col < 8; col++) {
        if (
            auto const& opt = board.at(Square {col, from_row});
            opt.has_value() && opt.value() == piece
        ) {
            Move mv = {{col, from_row}, to, promotionTo};
            if (isMoveLegal(board, std::move(mv))) {
                return col;
            }
        }
    }
    return std::nullopt;
}

bool set_src_square(
    Board const& board, Piece const& piece, Square to, Square& from,
    bool srcRowAlreadyKnown, bool srcColAlreadyKnown,
    std::optional<Piece::Type> const& promotionTo
) {

    VLOG(6) << "setting src square for piece " << piece << " to " << to;
    if (srcRowAlreadyKnown && srcColAlreadyKnown) {
        return true;
    }
    if (!srcRowAlreadyKnown && !srcColAlreadyKnown) {
        std::optional<Square> src = find_src_square(board, piece, to, promotionTo);
        if (src.has_value()) {
            from = src.value();
        }
        return src.has_value();
    }
    if (!srcRowAlreadyKnown) {
        std::optional<std::uint8_t> src_rank = find_src_row(board, piece, to, from.col, promotionTo);
        if (src_rank.has_value()) {
            from.row = src_rank.value();
        }
        return src_rank.has_value();
    }
    if (!srcColAlreadyKnown) {
        std::optional<std::uint8_t> src_file = find_src_col(board, piece, to, from.row, promotionTo);
        if (src_file.has_value()) {
            from.col = src_file.value();
        }
        return src_file.has_value();
    }
    throw std::runtime_error("Should not reach this part of the code");
}

std::optional<std::pair<std::vector<Game::MoveWithContext>, std::optional<ResultType>>> parseMovesAndResult(
    std::string const& pgn, std::size_t i, Board& board
) {
    std::vector<Game::MoveWithContext> moves {};
    std::optional<ResultType> result {};

    auto skip_comment = [](std::size_t i, std::string const& pgn) {
        VLOG(6) << "asked to skip comment from i=" << i;
        if (i >= pgn.size()) {
            return i;
        }
        if (pgn.at(i) == '{') {
            i++;
            while (i < pgn.size() && pgn.at(i) != '}') {
                i++;
            }
            i++;
            VLOG(7) << "returning i = " << i << " after skipping block comment";
            return i;
        }
        if (pgn.at(i) == ';') {
            i++;
            while (i < pgn.size() && pgn.at(i) != '\n') {
                i++;
            }
            i++;
            VLOG(7) << "returning i = " << i << " after skipping line comment";
            return i;
        }
        VLOG(7) << "didnt skip any chars for comments";
        return i;
    };
    auto skip_spacy = [](std::size_t i, std::string const& pgn) {
        VLOG(4) << "asked to skip space from i=" << i;
        while (i < pgn.size() && is_spacy(pgn.at(i))) {
            i++;
        }
        VLOG(7) << "returning i = " << i << " after skipping chars for comments";
        return i;
    };
    auto skip_to_legit_char = [skip_spacy, skip_comment] (std::size_t i, std::string const& pgn) {
        while (i < pgn.size()) {
            if (is_spacy(pgn.at(i))) {
                i = skip_spacy(i, pgn);
                continue;
            }
            if (pgn.at(i) == '{' || pgn.at(i) == ';') {
                i = skip_comment(i, pgn);
                continue;
            }
            return i;
        }
        return i;
    };

    auto parse_move = [] (
        std::size_t i, std::string const& pgn, Color color, Board& board
    ) -> std::optional<std::pair<std::size_t, Game::MoveWithContext>> {
        Square from {0,0};
        Square to {0,0};
        Piece::Type type = Piece::Type::Pawn;
        bool isCapture = false;
        bool isCheck = false;
        bool isCheckmate = false;
        bool isSrcFileAmbigious = false;
        bool isSrcRankAmbigious = false;
        std::optional<Side> isCastle = std::nullopt;
        std::optional<Piece::Type> promotionTo = std::nullopt;
        if (i >= pgn.size()) {
            return std::nullopt;
        }
        VLOG(3) << "asked to parse move from next few chars: " << pgn.substr(i, 7);
        std::size_t init_i = i;
        while(is_non_spacy(pgn.at(i))) {
            i++;
        }
        std::string chunk = pgn.substr(init_i, i-init_i);
        VLOG(3) << "chunk = " << chunk;
        if (chunk.size() < 2) {
            VLOG(3) << "got chunk smaller than 2 chars";
            return std::nullopt;
        }
        std::size_t j=chunk.size()-1;
        char c = chunk.at(j);
        if (c == '#') {
            isCheckmate = true;
            j--;
        } else if (c == '+') {
            isCheck = true;
            j--;
        }

        if (chunk.substr(0,j+1) == "O-O" || chunk.substr(0, j+1) == "O-O-O") {
            type = Piece::Type::King;
            from.col = 4;
            from.row = color == Color::White ? 0 : 7;
            to.row = from.row;
            if (chunk.substr(0, j+1) == "O-O-O") {
                to.col = 2;
                isCastle = std::make_optional(Side::QueenSide);
            } else {
                to.col = 6;
                isCastle = std::make_optional(Side::KingSide);
            }
            Piece piece {type, color};
            Move move {from, to, promotionTo};
            Game::MoveWithContext mv {move, piece, isCapture, isCheck, isCheckmate, isSrcFileAmbigious, isSrcRankAmbigious, isCastle};
            bool is_valid = makeMove(board, move);
            if (!is_valid) {
                VLOG(3) << "castling determined to be illegal";
                return std::nullopt;
            }
            return std::pair(i, mv);
        }

        c = chunk.at(j);
        if ((c == 'Q') || (c == 'N') || (c == 'R') || (c == 'B')) {
            VLOG(7) << "parsing promotion piece from char " << c;
            promotionTo = std::make_optional(from_pgn_to_piece_type(c));
            j--;
            if (chunk.at(j) != '=') {
                VLOG(3) << "expected equals symbol to signal promotion, return nullopt";
                return std::nullopt;
            }
            j--;
        }

        c = chunk.at(j);
        if (!std::isdigit(c)) {
            VLOG(3) << "didnt get correct to row digit";
            return std::nullopt;
        }
        to.row = c - '1';
        j--;

        c = chunk.at(j);
        if (!(c >= 'a' && c <= 'h')) {
            VLOG(3) << "didnt get correct to col alphabet";
            return std::nullopt;
        }
        to.col = c - 'a';
        j--;

        if (j < chunk.size() && chunk.at(j) == 'x') {
            VLOG(7) << "parsing capture";
            isCapture = true;
            j--;
        }
        if (j < chunk.size() && std::isdigit(chunk.at(j))) {
            VLOG(7) << "parsing src row";
            from.row = chunk.at(j) - '1';
            isSrcRankAmbigious = true;
            j--;
        }
        if (j < chunk.size() && chunk.at(j) >= 'a' && chunk.at(j) <= 'h') {
            VLOG(7) << "parsing src col";
            from.col = chunk.at(j) - 'a';
            isSrcFileAmbigious = true;
            j--;
        }
        if (j < chunk.size() && std::isalpha(chunk.at(j))) {
            VLOG(7) << "parsing type from char from chunk=" << chunk << " at j=" << j << " which is " << chunk.at(j);
            type = from_pgn_to_piece_type(chunk.at(j));
            j--;
        }

        if (j < chunk.size()) {
            VLOG(3) << "Expected to have processed all chars in the chunk but more left";
            return std::nullopt;
        }

        Piece piece {type, color};
        if (!set_src_square(board, piece, to, from, isSrcRankAmbigious, isSrcFileAmbigious, promotionTo)) {
            VLOG(3) << "Couldnt infer the source square for move";
            return std::nullopt;
        }
        Move move {from, to, promotionTo};
        Game::MoveWithContext mv {move, piece, isCapture, isCheck, isCheckmate, isSrcFileAmbigious, isSrcRankAmbigious, isCastle};
        bool is_valid = makeMove(board, move);
        if (!is_valid) {
            VLOG(3) << "move determined to be illegal" << std::endl;
            return std::nullopt;
        }
        return std::pair(i, mv);
    };

    auto parse_potential_result = [] (std::size_t i, std::string const& pgn) -> std::optional<ResultType> {
        VLOG(7) << "parsing result, potentially";
        if (i >= pgn.size()) {
            return std::nullopt;
        }
        if (pgn.substr(i, 3) == "1-0") {
            return ResultType::WhiteWin;
        }
        if (pgn.substr(i, 3) == "0-1") {
            return ResultType::BlackWin;
        }
        if (pgn.substr(i, 7) == "1/2-1/2") {
            return ResultType::Draw;
        }
        return std::nullopt;
    };

    VLOG(2) << "parsing moves, i=" << i << ", pgn.size()=" << pgn.size();
    while (i < pgn.size()) {
        VLOG(4) << "at i=" << i;
        i = skip_to_legit_char(i, pgn);
        VLOG(4) << "next few chars: " << pgn.substr(i, 7);
        if (i >= pgn.size()) {
            VLOG(2) << "ran out of chars, i=" << i;
            break;
        }
        result = parse_potential_result(i, pgn);
        if (result.has_value()) {
            break;
        }
        if (!std::isdigit(pgn.at(i))) {
            VLOG(2) << "couldnt find digit for move number";
            return std::nullopt;
        }
        std::size_t move_num = 0;
        while (std::isdigit(pgn.at(i))) {
            move_num = move_num*10 + pgn.at(i) - '0';
            i++;
        }
        if (pgn.at(i) != '.') {
            VLOG(2) << "didnt get dot after move number";
            return std::nullopt;
        }
        i++;

        VLOG(2) << "parsing move number " << move_num;
        std::size_t i_prev = i;
        i = skip_to_legit_char(i, pgn);
        if (i == i_prev) {
            VLOG(2) << "expected some space between move number and move";
            return std::nullopt;
        }
        if (i >= pgn.size()) {
            VLOG(2) << "ran out of pgn text";
            return std::nullopt;
        }

        std::optional<std::pair<std::size_t, Game::MoveWithContext>> res = parse_move(i, pgn, Color::White, board);
        if (!res.has_value()) {
            VLOG(2) << "failed to parse whites move successfully";
            return std::nullopt;
        }
        i = res.value().first;
        moves.push_back(res.value().second);

        if(i<pgn.size() && std::isalnum(pgn.at(i))) {
            VLOG(2) << "expected space between black and whites moves";
            return std::nullopt;
        }

        i = skip_to_legit_char(i, pgn);
        if (i >= pgn.size()) {
            VLOG(2) << "ran out of pgn text";
            break;
        }
        result = parse_potential_result(i, pgn);
        if (result.has_value()) {
            break;
        }

        res = parse_move(i, pgn, Color::Black, board);
        if (!res.has_value()) {
            VLOG(2) << "failed to parse blacks move successfully";
            return std::nullopt;
        }
        i = res.value().first;
        moves.push_back(res.value().second);
        if(i<pgn.size() && std::isalnum(pgn.at(i))) {
            VLOG(2) << "expected space after blacks move but got: " << pgn.at(i);
            return std::nullopt;
        }
    }
    if (!moves.empty() && moves.back().isCheckmate) {
        if (moves.back().piece.color == Color::White) {
            result = ResultType::WhiteWin;
        } else {
            result = ResultType::BlackWin;
        }
    }
    return std::make_optional(std::make_pair(moves, result));
}

std::string to_pgn_string(std::optional<ResultType> result) {
    if (!result.has_value()) {
        return "*";
    }
    switch (result.value()) {
        case ResultType::Draw:
            return "1/2-1/2";
        case ResultType::WhiteWin:
            return "1-0";
        case ResultType::BlackWin:
            return "0-1";
    }
    throw std::logic_error("Bad switch statement");
}

std::string to_pgn_string(Game::SevenTagRoster const& roster) {
    std::string result {};
    result += "[Event \"" + roster.event + "\"]\n";
    result += "[Site \"" + roster.site + "\"]\n";
    result += "[Date \"" + roster.date + "\"]\n";
    result += "[Round \"" + roster.round + "\"]\n";
    result += "[White \"" + roster.white + "\"]\n";
    result += "[Black \"" + roster.black + "\"]\n";
    result += "[Result \"" + to_pgn_string(roster.result) + "\"]\n";
    return result;
}

char to_pgn_char(Piece::Type type) {
    switch (type) {
        case Piece::Type::King:
            return 'K';
        case Piece::Type::Queen:
            return 'Q';
        case Piece::Type::Bishop:
            return 'B';
        case Piece::Type::Knight:
            return 'N';
        case Piece::Type::Rook:
            return 'R';
        case Piece::Type::Pawn:
        default:
            throw std::logic_error("shouldnt have been called");
    }
}

std::string to_pgn_string(Game::MoveWithContext const& move) {
    std::string result {};
    if (move.isCastle.has_value()) {
        if (move.isCastle.value() == Side::KingSide) {
            result += "O-O";
        } else {
            result += "O-O-O";
        }
    } else {
        if (move.piece.type != Piece::Type::Pawn) {
            result += to_pgn_char(move.piece.type);
        }
        if (move.isSrcFileAmbigious) {
            result += move.move.fromSquare.pgn_file();
        }
        if (move.isSrcRankAmbigious) {
            result += move.move.fromSquare.pgn_rank();
        }
        if (move.isCapture) {
            result += 'x';
        }
        result += move.move.toSquare.pgn_file();
        result += move.move.toSquare.pgn_rank();
        if (move.move.promotionTo.has_value()) {
            result += "=";
            result += to_pgn_char(move.move.promotionTo.value());
        }
    }
    if (move.isCheck) {
        result += '+';
    }
    if (move.isCheckmate) {
        result += '#';
    }
    return result;
}

std::string to_pgn_string(std::vector<Game::MoveWithContext> const& moves) {
    std::string result {};
    for (std::size_t i = 0; i < moves.size(); i++) {
        if (i%2 == 0) {
            result += std::to_string((i/2) + 1);
            result += ". ";
        }

        Game::MoveWithContext const& mv = moves.at(i);
        result += to_pgn_string(mv);

        if (i == moves.size()-1) {
            continue;
        }
        if (i%20 == 19) {
            result += "\n";
        } else {
            result += " ";
        }
    }
    return result;
}
}

namespace ChessEngineLib {

Game::Game()
: roster_ {},
moves_{},
result_ {std::nullopt},
board_ {Board::startingPosBoard()}
{}

std::optional<Game> Game::fromPgn(std::string const& pgn) {
    VLOG(2) << "fromPgn called with pgn of size = " << pgn.size();
    std::optional<std::pair<SevenTagRoster, std::size_t>> roster = parseRoster(pgn);
    if (!roster.has_value()) {
        return std::nullopt;
    }

    std::size_t i = roster.value().second;
    VLOG(2) << "calling parseMoves from i = " << i;

    Game game {};
    auto moves_optional = parseMovesAndResult(pgn, i, game.board_);
    if (!moves_optional.has_value()) {
        return std::nullopt;
    }

    game.roster_ = roster.value().first;
    game.moves_ = moves_optional.value().first;
    game.result_ = moves_optional.value().second;
    if (game.result_.has_value()) {
        game.roster_.result = game.result_;
    }
    return game;
}

std::string Game::toPgn() const {
    std::string sevenTagRosterStr = to_pgn_string(roster_);
    std::string movesPgnStr = to_pgn_string(moves_);
    if (result_.has_value()) {
        movesPgnStr += " " + to_pgn_string(result_);
    }
    return sevenTagRosterStr + "\n" + movesPgnStr + "\n";
}

Game::SevenTagRoster const& Game::sevenTagRoster() const {
    return roster_;
}

std::optional<ResultType> Game::result() const {
    return result_;
}

std::size_t Game::movesSize() const {
    return moves_.size();
}

std::optional<Game::MoveWithContext> Game::moveAt(std::size_t moveNum, Color color) const {
    return moveAt((moveNum-1)*2 + (color == Color::Black ? 2:1));
}

std::optional<Game::MoveWithContext> Game::moveAt(std::size_t halfMoveNum) const {
    if (halfMoveNum-1 >= moves_.size()) {
        return std::nullopt;
    }
    return moves_.at(halfMoveNum-1);
}

Board const& Game::board() const {
    return board_;
}

Game::MoveWithContext::MoveWithContext(
    Move const& move,
    Piece const& piece,
    bool is_capture,
    bool is_check,
    bool is_checkmate,
    bool is_src_file_ambig,
    bool is_src_rank_ambig,
    std::optional<Side> is_castle
):
move {move},
piece {piece},
isCapture(is_capture),
isCheck(is_check),
isCheckmate(is_checkmate),
isSrcFileAmbigious(is_src_file_ambig),
isSrcRankAmbigious(is_src_rank_ambig),
isCastle(is_castle)
{}

}

