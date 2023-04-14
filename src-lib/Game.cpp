#include "Game.hpp"
#include "GameEngine.hpp"
#include "Move.hpp"
#include "glog/logging.h"
#include <cassert>
#include <cctype>
#include <optional>

namespace {

using namespace ChessEngineLib;

std::vector<std::string> split_string(
    const std::string & x,
    const std::string delim
) {
    std::vector<std::string> chunks {};
    auto start = 0U;
    auto end = x.find(delim);
    while (end != std::string::npos) {
        chunks.push_back(x.substr(start, end - start));
        start = end + delim.length();
        end = x.find(delim, start);
    }
    chunks.push_back(x.substr(start, end - start));
    return chunks;
}

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

std::optional<std::pair<std::vector<Game::MoveWithContext>, std::optional<ResultType>>> parseMovesAndResult(std::string const& pgn, std::size_t i) {
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

    auto parse_move = [] (std::size_t i, std::string const& pgn, Color color) -> std::optional<std::pair<std::size_t, Game::MoveWithContext>> {
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

        if (chunk.substr(0,j+1) == "O-O" || chunk.substr(0, j+1) == "0-0-0") {
            type = Piece::Type::King;
            from.col = 4;
            from.row = color == Color::White ? 0 : 7;
            to.row = from.row;
            if (chunk.substr(0, j+1) == "0-0-0") {
                to.col = 2;
                isCastle = std::make_optional(Side::QueenSide);
            } else {
                to.col = 6;
                isCastle = std::make_optional(Side::KingSide);
            }
            Piece piece {type, color};
            Move move {piece, from, to, promotionTo};
            Game::MoveWithContext mv {move, piece, isCapture, isCheck, isCheckmate, isSrcFileAmbigious, isSrcRankAmbigious, isCastle};
            return std::pair(i, mv);
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
            VLOG(7) << "parsing type";
            type = from_pgn_to_piece_type(pgn.at(j));
            j--;
        }

        if (j < chunk.size()) {
            VLOG(3) << "Expected to have processed all chars in the chunk but more left";
            return std::nullopt;
        }

        Piece piece {type, color};
        Move move {piece, from, to, promotionTo};
        Game::MoveWithContext mv {move, piece, isCapture, isCheck, isCheckmate, isSrcFileAmbigious, isSrcRankAmbigious, isCastle};
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

        std::optional<std::pair<std::size_t, Game::MoveWithContext>> res = parse_move(i, pgn, Color::White);
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

        res = parse_move(i, pgn, Color::Black);
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
    return std::make_optional(std::make_pair(moves, result));
}

}

namespace ChessEngineLib {

std::optional<Game> Game::fromPgn(std::string const& pgn) {
    (void) pgn;
    (void) split_string;
    VLOG(2) << "fromPgn called with pgn of size = " << pgn.size();
    std::optional<std::pair<SevenTagRoster, std::size_t>> roster = parseRoster(pgn);
    if (!roster.has_value()) {
        return std::nullopt;
    }

    std::size_t i = roster.value().second;
    VLOG(2) << "calling parseMoves from i = " << i;
    auto moves_optional = parseMovesAndResult(pgn, i);
    if (!moves_optional.has_value()) {
        return std::nullopt;
    }

    Game game {};
    game.roster_ = roster.value().first;
    game.moves_ = moves_optional.value().first;
    game.result_ = moves_optional.value().second;
    return game;
}

std::string Game::toPgn() const {
    return "";
}

Game::SevenTagRoster const& Game::sevenTagRoster() const {
    return roster_;
}

std::optional<ResultType> Game::result() const {
    return result_;
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

