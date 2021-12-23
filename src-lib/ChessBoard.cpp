#include <nlohmann/json.hpp>
#include "ChessBoard.hpp"

int getChessBoard() {
    return 64;
}

std::string prettyJson(std::string json) {
    return nlohmann::json::parse(json).dump(2);
}
