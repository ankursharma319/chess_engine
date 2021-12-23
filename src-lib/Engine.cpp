#include <nlohmann/json.hpp>
#include "Engine.hpp"

namespace ChessEngineLib {

bool validateFen(std::string fen) {
    (void) fen;
    return false;
}

bool isMoveLegal(std::string fen, std::string move) {
    (void) fen;
    (void) move;
    return false;
}

std::size_t getNumberOfLegalMoves(std::string fen) {
    (void) fen;
    return 0;
}

}
