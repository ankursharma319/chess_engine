#include <nlohmann/json.hpp>
#include "Board.hpp"

namespace {

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

}

namespace ChessEngineLib {

std::optional<Board> Board::fromFen(std::string fen) {
    (void) fen;
    std::vector<std::string> chunks = split_string(fen, " ");
    if (chunks.size() != 6) {
        return std::nullopt;
    }
    const std::vector<std::string> ranks = split_string(chunks.at(0), "/");
    if (ranks.size() != 8) {
        return std::nullopt;
    }
    return std::nullopt;
}

std::array<std::array<std::optional<Piece>, 8>, 8> Board::getContents() const {
    return m_grid;
}

Color Board::getNextMoveColor() const {
    return Color::Black;
}

bool Board::isCastlingExpired(Color color, Side side) const {
    (void) color;
    (void) side;
    return false;
}

unsigned short Board::getHalfMoveClock() const {
    return 0;
}

std::size_t Board::getMoveNumber() const {
    return 0;
}

std::optional<Square> Board::getEnPassantSquare() {
    return std::nullopt;
}

}
