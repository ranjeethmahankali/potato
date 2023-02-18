#include <Move.h>
#include <algorithm>
#include <glm/fwd.hpp>
#include <ostream>
#include <regex>

namespace potato {

bool HistoryData::operator==(const HistoryData& other) const
{
  return std::memcmp(this, &other, sizeof(HistoryData)) == 0;
}
bool HistoryData::operator!=(const HistoryData& other) const
{
  return !(*this == other);
}

HistoryData History::pop()
{
  HistoryData h = std::stack<HistoryData>::top();
  std::stack<HistoryData>::pop();
  return h;
}

Color color(Piece pc)
{
  return Color(pc & 0b1000);
}

PieceType type(Piece pc)
{
  return PieceType(pc & 0b111);
}

char symbol(Piece pc)
{
  static constexpr std::array<char, NUniquePieces> sSymbols = {
    {'_', 'p', 'n', 'b', 'r', 'q', 'k', '_', '_', 'P', 'N', 'B', 'R', 'Q', 'K'}};
  return sSymbols[pc];
}

Position::Position()
{
  clear();
  // Black pieces.
  put({{{0, Piece::B_ROK},
        {1, Piece::B_HRS},
        {2, Piece::B_BSH},
        {3, Piece::B_QEN},
        {4, Piece::B_KNG},
        {5, Piece::B_BSH},
        {6, Piece::B_HRS},
        {7, Piece::B_ROK}}});
  // Black pawns
  for (int i = 8; i < 16; ++i) {
    put(i, Piece::B_PWN);
  }
  // White pawns
  for (int i = 48; i < 56; ++i) {
    put(i, Piece::W_PWN);
  }
  // White pieces
  put({{{56, Piece::W_ROK},
        {57, Piece::W_HRS},
        {58, Piece::W_BSH},
        {59, Piece::W_QEN},
        {60, Piece::W_KNG},
        {61, Piece::W_BSH},
        {62, Piece::W_HRS},
        {63, Piece::W_ROK}}});
}

using ZobristTable = std::array<uint64_t, NUniquePieces * 64>;

static ZobristTable generateZobristTable()
{
  std::array<uint64_t, NUniquePieces * 64> table;
  for (uint64_t pc = 0; pc < NUniquePieces; ++pc) {
    for (uint64_t pos = 0; pos < 64; ++pos) {
      table[pc * 64 + pos] = uint64_t(std::rand()) | (uint64_t(std::rand()) << 32);
    }
  }
  return table;
}

static const ZobristTable& zobristTable()
{
  static const ZobristTable sTable = generateZobristTable();
  return sTable;
}

void Position::calcHash()
{
  mHash = 0x70329434d587dc75;  // seed.
  for (int i = 0; i < 64; ++i) {
    mHash ^= zobristTable()[mPieces[i] * 64 + i];
  }
}

Position& Position::put(int pos, Piece pc)
{
  Piece    old  = std::exchange(mPieces[pos], pc);
  BitBoard mask = OneHot[pos];
  mBitBoards[old] &= ~mask;
  mBitBoards[pc] |= mask;
  mHash ^= zobristTable()[old * 64 + pos] ^ zobristTable()[pc * 64 + pos];
  return *this;
}

Position& Position::put(std::span<const std::pair<int, Piece>> pieces)
{
  for (auto [pos, pc] : pieces) {
    put(pos, pc);
  }
  return *this;
}

Position& Position::put(glm::ivec2 pos, Piece pc)
{
  return put(pos.y * 8 + pos.x, pc);
}

Position& Position::remove(int pos)
{
  return put(pos, Piece::NONE);
}

Position& Position::remove(glm::ivec2 pos)
{
  return put(pos, Piece::NONE);
}

Position& Position::move(int from, int to)
{
  Piece pc = mPieces[from];
  return remove(from).put(to, pc);
}

Position& Position::move(glm::ivec2 from, glm::ivec2 to)
{
  return move(from.y * 8 + from.x, to.y * 8 + to.x);
}

Piece Position::piece(int pos) const
{
  return mPieces[pos];
}

Piece Position::piece(glm::ivec2 pos) const
{
  return piece(pos.y * 8 + pos.x);
}

BitBoard Position::board(Piece p) const
{
  return mBitBoards[p];
}

int Position::enpassantSq() const
{
  return mEnPassantSquare;
}

Castle Position::castlingRights() const
{
  return mCastlingRights;
}

void Position::setEnpassantSq(int enp)
{
  mEnPassantSquare = enp;
}
void Position::setCastlingRights(Castle c)
{
  mCastlingRights = c;
}

Color Position::turn() const
{
  return mTurn;
}

void Position::setTurn(Color turn)
{
  mTurn = turn;
}

void Position::clear()
{
  std::fill(mPieces.begin(), mPieces.end(), Piece::NONE);
  std::fill(mBitBoards.begin(), mBitBoards.end(), 0);
  mBitBoards[Piece::NONE] = 0xffffffffffffffff;  // All squares contain the NONE piece.
  mHalfMoves              = 0;
  mMoveCounter            = 1;
  mEnPassantSquare        = -1;
  mTurn                   = Color::WHT;
  calcHash();
}

size_t Position::hash() const
{
  return mHash;
}

bool Position::operator==(const Position& other) const
{
  return mPieces == other.mPieces && mBitBoards == other.mBitBoards &&
         mHistory == other.mHistory && mHash == other.mHash &&
         mHalfMoves == other.mHalfMoves && mMoveCounter == other.mMoveCounter &&
         mEnPassantSquare == other.mEnPassantSquare &&
         mCastlingRights == other.mCastlingRights && mTurn == other.mTurn;
}

bool Position::operator!=(const Position& other) const
{
  return !(*this == other);
}

Position Position::empty()
{
  Position p;
  p.clear();
  return p;
}

int fileToX(char file)
{
  switch (file) {
  case 'a':
    return 0;
  case 'b':
    return 1;
  case 'c':
    return 2;
  case 'd':
    return 3;
  case 'e':
    return 4;
  case 'f':
    return 5;
  case 'g':
    return 6;
  case 'h':
    return 7;
  default:
    return -1;
  }
}

int rankToY(char rank)
{
  switch (rank) {
  case '1':
    return 7;
  case '2':
    return 6;
  case '3':
    return 5;
  case '4':
    return 4;
  case '5':
    return 3;
  case '6':
    return 2;
  case '7':
    return 1;
  case '8':
    return 0;
  default:
    return -1;
  }
}

using SubMatch = std::sub_match<std::string::const_iterator>;
static void parsePlacement(const SubMatch& placement, Position& b)
{
  int pos = 0;
  // auto       shift = [&pos](int offset = 1) {
  //   int flat = pos.x + 8 * pos.y;
  //   flat += offset;
  //   pos = {flat % 8, flat / 8};
  // };
  for (auto it = placement.first; it != placement.second; ++it) {
    char c = *it;
    switch (c) {
    case 'p':
      b.put(pos, B_PWN);
      ++pos;
      break;
    case 'n':
      b.put(pos, B_HRS);
      ++pos;
      break;
    case 'b':
      b.put(pos, B_BSH);
      ++pos;
      break;
    case 'r':
      b.put(pos, B_ROK);
      ++pos;
      break;
    case 'q':
      b.put(pos, B_QEN);
      ++pos;
      break;
    case 'k':
      b.put(pos, B_KNG);
      ++pos;
      break;
    case 'P':
      b.put(pos, W_PWN);
      ++pos;
      break;
    case 'N':
      b.put(pos, W_HRS);
      ++pos;
      break;
    case 'B':
      b.put(pos, W_BSH);
      ++pos;
      break;
    case 'R':
      b.put(pos, W_ROK);
      ++pos;
      break;
    case 'Q':
      b.put(pos, W_QEN);
      ++pos;
      break;
    case 'K':
      b.put(pos, W_KNG);
      ++pos;
      break;
    case '/':
      if (pos % 8) {
        throw std::logic_error("Error when parsing the fen string");
      }
      break;
    case '1':
      ++pos;
      break;
    case '2':
      pos += 2;
      break;
    case '3':
      pos += 3;
      break;
    case '4':
      pos += 4;
      break;
    case '5':
      pos += 5;
      break;
    case '6':
      pos += 6;
      break;
    case '7':
      pos += 7;
      break;
    case '8':
      pos += 8;
      break;
    }
  }
}

static Color parseActiveColor(const SubMatch& rTurn)
{
  if (rTurn.length() != 1) {
    throw std::logic_error("Invalid active color field in the fen string");
  }
  char c = *rTurn.first;
  if (c == 'w') {
    return Color::WHT;
  }
  else if (c == 'b') {
    return Color::BLK;
  }
  else {
    throw std::logic_error("Invalid active color field in the fen string");
  }
}

static void parseCastlingRights(const SubMatch& castling, Castle& rights)
{
  if (castling.length() > 4 || castling.length() < 1) {
    throw std::logic_error("Invalid castling rights field in the fen string.");
  }
  static constexpr std::array<std::tuple<char, Castle>, 4> sCastlingPos = {{
    {'K', Castle::W_SHORT},
    {'Q', Castle::W_LONG},
    {'k', Castle::B_SHORT},
    {'q', Castle::B_LONG},
  }};
  rights                                                                = Castle(0);
  for (auto it = castling.first; it != castling.second; ++it) {
    char c     = *it;
    auto match = std::find_if(sCastlingPos.begin(), sCastlingPos.end(), [c](auto tup) {
      return std::get<0>(tup) == c;
    });
    if (match == sCastlingPos.end()) {
      continue;
    }
    rights = Castle(rights | std::get<1>(*match));
  }
}

static void parseEnpassant(const SubMatch& enpassant, int& enp)
{
  if (enpassant.length() > 2 || enpassant.length() < 1) {
    throw std::logic_error("Invalid enpassant target square field in the fen string");
  }
  if (enpassant == "-") {
    enp = -1;
  }
  else {
    enp = fileToX(*enpassant.first) + 8 * rankToY(*(enpassant.first + 1));
  }
}

Position Position::fromFen(const std::string& fen)
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
    throw std::runtime_error("Failed to parse the above fen string");
  }
  Position board;
  board.clear();
  parsePlacement(results[1], board);
  board.mTurn = parseActiveColor(results[2]);
  parseCastlingRights(results[3], board.mCastlingRights);
  parseEnpassant(results[4], board.mEnPassantSquare);
  board.mHalfMoves   = std::stoi(results[5]);
  board.mMoveCounter = std::stoi(results[6]);
  return board;
}

std::string Position::fen() const
{
  std::string out;
  {  // Placement
    int empty = 0;
    for (size_t pi = 0; pi < mPieces.size(); ++pi) {
      Piece pc = mPieces[pi];
      if (pi % 8 == 0) {
        if (empty) {
          out += std::to_string(empty);
          empty = 0;
        }
        if (pi) {
          out.push_back('/');
        }
      }
      if (pc) {
        if (empty) {
          out += std::to_string(empty);
          empty = 0;
        }
        out.push_back(symbol(pc));
      }
      else {
        ++empty;
      }
    }
  }
  {  // Active turn
    out.push_back(mTurn == BLK ? 'b' : 'w');
  }
  {  // Castling
  }
  throw std::logic_error("Not Implemented.");
}

History& Position::history()
{
  return mHistory;
}

bool Position::valid() const
{
  std::array<int, NUniquePieces> counts;
  std::fill(counts.begin(), counts.end(), 0);
  for (Piece pc : mPieces) {
    ++counts[pc];
  }
  if (counts[7] & counts[8]) {
    return false;
  }
  for (size_t i = 0; i < NUniquePieces; ++i) {
    Piece pc       = Piece(i);
    int   expected = std::popcount(mBitBoards[pc]);
    if (counts[pc] != expected) {
      return false;
    }
  }
  return true;
}

Position& currentPosition()
{
  static Position sState;
  return sState;
}

void writeBoard(BitBoard b, std::ostream& os)
{
  for (int pos = 0; pos < 64; ++pos) {
    if (pos % 8 == 0) {
      if (pos) {
        os << '\n';
      }
      os << '|';
    }
    os << ((b & OneHot[pos]) ? 'X' : '_') << '|';
  }
  os << std::endl << std::endl;
}

}  // namespace potato

namespace std {
using namespace potato;
ostream& operator<<(ostream& os, Color c)
{
  os << (c == WHT ? "White" : c == BLK ? "Black" : "");
  return os;
}

ostream& operator<<(ostream& os, Castle c)
{
  if (c & Castle::W_SHORT) {
    os << 'K';
  }
  if (c & Castle::W_LONG) {
    os << 'Q';
  }
  if (c & Castle::B_SHORT) {
    os << 'k';
  }
  if (c & Castle::B_LONG) {
    os << 'q';
  }
  if (!c) {
    os << '-';
  }
  return os;
}

ostream& operator<<(ostream& os, const Position& b)
{
  for (int pos = 0; pos < 64; ++pos) {
    if (pos % 8 == 0) {
      if (pos) {
        os << '\n';
      }
      os << '|';
    }
    os << symbol(b.piece(pos)) << '|';
  }
  os << std::endl;
  os << "Turn: " << b.turn() << std::endl;
  os << "Castling rights: " << b.castlingRights() << std::endl;
  return os;
}

}  // namespace std
