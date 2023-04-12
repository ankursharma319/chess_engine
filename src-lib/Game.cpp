#include "Game.hpp"
#include "GameEngine.hpp"
#include "glog/logging.h"
#include <cassert>
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

bool is_non_spacy(char c) {
    return c != ' ' && c!= '\n' && c!='\t';
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

std::optional<std::vector<Game::MoveWithContext>> parseMoves(std::string const& pgn, std::size_t i) {
    while (i < pgn.size()) {
        i++;
    }
    return std::nullopt;
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

}

