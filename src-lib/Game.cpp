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
    return c == ' ' || c == '\n' || c!='\t';
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

std::optional<std::vector<Game::MoveWithContext>> parseMoves(std::string const& pgn, std::size_t i) {
    std::vector<Game::MoveWithContext> moves {};
    auto skip_comment = [](std::size_t i, std::string const& pgn) {
        if (i >= pgn.size()) {
            return i;
        }
        if (pgn.at(i) == '{') {
            i++;
            while (i < pgn.size() && pgn.at(i) != '}') {
                i++;
            }
            return i;
        }
        if (pgn.at(i) == ';') {
            i++;
            while (i < pgn.size() && pgn.at(i) != '\n') {
                i++;
            }
            return i;
        }
        return i;
    };
    auto skip_spacy = [](std::size_t i, std::string const& pgn) {
        while (i < pgn.size() && is_spacy(pgn.at(i))) {
            i++;
        }
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
        bool is_valid_move = false;
        Square from;
        Square to;
        Piece::Type type;
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
        if (std::isalpha(pgn.at(i))) {
            type = from_pgn_to_piece_type(pgn.at(i));
            if (type != Piece::Type::Pawn) {
                i++;
            }
        } else {
            return std::nullopt;
        }

        if (i >= pgn.size()) {
            return std::nullopt;
        }
        if (std::isalpha(pgn.at(i))) {
            isSrcFileAmbigious = true;
            to.col = pgn.at(i) - 'a';
            i++;
        } else {
            return std::nullopt;
        }

        if (i >= pgn.size()) {
            return std::nullopt;
        }
        if (std::isdigit(pgn.at(i))) {
            isSrcRankAmbigious = true;
            to.row = pgn.at(i) - '1';
            i++;
        } else {
            return std::nullopt;
        }

        if (i < pgn.size() && std::isalnum(pgn.at(i))) {
            from = to;
            if (std::isalpha(pgn.at(i))) {
                to.col = pgn.at(i) - 'a';
                i++;
            }
            if (i < pgn.size() && std::isdigit(pgn.at(i))) {
                to.row = pgn.at(i) - '1';
                i++;
            }
        } else {
            isSrcFileAmbigious = false;
            isSrcRankAmbigious = false;
        }

        if (is_valid_move) {
            Piece piece {type, color};
            Move move {piece, from, to, promotionTo};
            Game::MoveWithContext mv {move, piece, isCapture, isCheck, isCheckmate, isSrcFileAmbigious, isSrcRankAmbigious, isCastle};
            return std::pair(i, mv);
        }
        return std::nullopt;
    };

    while (i < pgn.size()) {
        i = skip_to_legit_char(i, pgn);
        if (i >= pgn.size()) {
            break;
        }
        if (!std::isdigit(pgn.at(i))) {
            return std::nullopt;
        }
        std::size_t move_num = 0;
        while (std::isdigit(pgn.at(i))) {
            move_num = move_num*10 + pgn.at(i) - '0';
            i++;
        }
        std::size_t i_prev = i;
        i = skip_to_legit_char(i, pgn);
        if (i == i_prev) {
            return std::nullopt;
        }
        if (i >= pgn.size()) {
            break;
        }
        std::optional<std::pair<std::size_t, Game::MoveWithContext>> res = parse_move(i, pgn, Color::White);
        if (!res.has_value()) {
            return std::nullopt;
        }
        i = res.value().first;
        moves.push_back(res.value().second);

        if(i<pgn.size() && std::isalnum(pgn.at(i))) {
            return std::nullopt;
        }

        i = skip_to_legit_char(i, pgn);
        if (i >= pgn.size()) {
            break;
        }
        res = parse_move(i, pgn, Color::Black);
        if (!res.has_value()) {
            return std::nullopt;
        }
        i = res.value().first;
        moves.push_back(res.value().second);
        if(i<pgn.size() && std::isalnum(pgn.at(i))) {
            return std::nullopt;
        }
    }
    return std::make_optional(moves);
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
    auto moves_optional = parseMoves(pgn, i);
    if (!moves_optional.has_value()) {
        return std::nullopt;
    }

    Game game {};
    game.roster_ = roster.value().first;
    game.moves_ = moves_optional.value();
    return game;
}

std::string Game::toPgn() const {
    return "";
}

Game::SevenTagRoster const& Game::sevenTagRoster() const {
    return roster_;
}

std::optional<Game::MoveWithContext> Game::moveAt(std::size_t moveNum, Color color) const {
    (void) color;
    (void) moveNum;
    return std::nullopt;
}

std::optional<Game::MoveWithContext> Game::moveAt(std::size_t halfMoveNum) const {
    (void) halfMoveNum;
    return std::nullopt;
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

