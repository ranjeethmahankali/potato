#include <Board.h>
#include <iostream>
#include <regex>
#include <stdexcept>

namespace potato {
using SubMatch = std::sub_match<std::string::const_iterator>;
static void parsePlacement(const SubMatch& placement, Board& b)
{
  glm::ivec2 pos   = {0, 0};
  auto       shift = [&pos](int offset = 1) {
    int flat = pos.x + 8 * pos.y;
    flat += offset;
    pos = {flat % 8, flat / 8};
  };
  for (auto it = placement.first; it != placement.second; ++it) {
    char c = *it;
    switch (c) {
    case 'p':
      b.setPiece(pos, Piece::BLK | Piece::PWN);
      shift();
      break;
    case 'n':
      b.setPiece(pos, Piece::BLK | Piece::HRS);
      shift();
      break;
    case 'b':
      b.setPiece(pos, Piece::BLK | Piece::BSH);
      shift();
      break;
    case 'r':
      b.setPiece(pos, Piece::BLK | Piece::ROK);
      shift();
      break;
    case 'q':
      b.setPiece(pos, Piece::BLK | Piece::QEN);
      shift();
      break;
    case 'k':
      b.setPiece(pos, Piece::BLK | Piece::KNG);
      shift();
      break;
    case 'P':
      b.setPiece(pos, Piece::WHT | Piece::PWN);
      shift();
      break;
    case 'N':
      b.setPiece(pos, Piece::WHT | Piece::HRS);
      shift();
      break;
    case 'B':
      b.setPiece(pos, Piece::WHT | Piece::BSH);
      shift();
      break;
    case 'R':
      b.setPiece(pos, Piece::WHT | Piece::ROK);
      shift();
      break;
    case 'Q':
      b.setPiece(pos, Piece::WHT | Piece::QEN);
      shift();
      break;
    case 'K':
      b.setPiece(pos, Piece::WHT | Piece::KNG);
      shift();
      break;
    case '/':
      if (pos.x != 0) {
        throw std::logic_error("Error when parsing the fen string");
      }
      break;
    case '1':
      shift(1);
      break;
    case '2':
      shift(2);
      break;
    case '3':
      shift(3);
      break;
    case '4':
      shift(4);
      break;
    case '5':
      shift(5);
      break;
    case '6':
      shift(6);
      break;
    case '7':
      shift(7);
      break;
    case '8':
      shift(8);
      break;
    }
  }
}

static uint8_t parseActiveColor(const SubMatch& rTurn)
{
  if (rTurn.length() != 1) {
    throw std::logic_error("Invalid active color field in the fen string");
  }
  char c = *rTurn.first;
  if (c == 'w') {
    return Piece::WHT;
  }
  else if (c == 'b') {
    return Piece::BLK;
  }
  else {
    throw std::logic_error("Invalid active color field in the fen string");
  }
}

static void parseCastlingRights(const SubMatch& castling, Board& b)
{
  if (castling.length() > 4 || castling.length() < 1) {
    throw std::logic_error("Invalid castling rights field in the fen string.");
  }
  static constexpr std::array<std::tuple<char, glm::ivec2, glm::ivec2>, 4> sCastlingPos =
    {{
      {'K', {4, 7}, {7, 7}},
      {'Q', {4, 7}, {0, 7}},
      {'k', {4, 0}, {7, 0}},
      {'q', {4, 0}, {0, 0}},
    }};
  for (auto it = castling.first; it != castling.second; ++it) {
    char c     = *it;
    auto match = std::find_if(sCastlingPos.begin(), sCastlingPos.end(), [c](auto tup) {
      return std::get<0>(tup) == c;
    });
    auto [c2, kpos, rpos] = *match;
    b.setMask(kpos, Piece::CASTLE);
    b.setMask(rpos, Piece::CASTLE);
  }
}

static void parseEnpassant(const SubMatch& enpassant, Board& b)
{
  if (enpassant.length() > 2 || enpassant.length() < 1) {
    throw std::logic_error("Invalid enpassant target square field in the fen string");
  }
  b.setMask(glm::ivec2 {fileToX(*enpassant.first), rankToY(*(enpassant.first + 1))},
            Piece::ENPASSANT);
}

State State::fromFen(const std::string& fen)
{
  std::smatch results;
  if (std::regex_search(fen,
                        results,
                        std::regex("([p,P,n,N,b,B,r,R,q,Q,k,K,1-8,/]+)\\s"  // placement
                                   "([b,w])\\s"                             // turn
                                   "([K,Q,k,q,-]+)\\s"  // Castling rights
                                   "([a-h,1-8,-]+)\\s"  // Enpassant target squares
                                   "(\\d+)\\s"          // Half moves
                                   "(\\d+)"             // Full moves.
                                   ))) {}
  else {
    std::cerr << fen << std::endl;
    throw std::runtime_error("Failed to parse the above fen string");
  }
  State state;
  state.mBoard.clear();
  parsePlacement(results[1], state.mBoard);
  state.mTurn = parseActiveColor(results[2]);
  parseCastlingRights(results[3], state.mBoard);
  parseEnpassant(results[4], state.mBoard);
  state.mHalfMoves = std::stoi(results[5]);
  state.mFullMoves = std::stoi(results[6]);
  return state;
}

State& currentState()
{
  static State sState;
  return sState;
}

}  // namespace potato
