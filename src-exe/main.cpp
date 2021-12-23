#include <iostream>
#include <nlohmann/json.hpp>
#include "ChessEngineLib/ChessBoard.hpp"

int main() {
    nlohmann::json my_json {{"json_key", getChessBoard()}};
    std::cout << "Hello World!" << std::endl;
    std::cout << my_json.dump(2) << std::endl;
    return 0;
}
