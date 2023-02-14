#include <Move.h>
#include <Position.h>
#include <algorithm>
#include <glm/fwd.hpp>
#include <regex>

namespace potato {

static constexpr std::array<glm::ivec2, 8> sHorseMoves   = {{
    {1, 2},
    {2, 1},
    {-1, 2},
    {-2, 1},
    {1, -2},
    {2, -1},
    {-1, -2},
    {-2, -1},
}};
static constexpr std::array<glm::ivec2, 4> sDiagonalDirs = {{
  {1, 1},
  {-1, 1},
  {1, -1},
  {-1, -1},
}};
static constexpr std::array<glm::ivec2, 4> sAxialDirs    = {{
     {1, 0},
     {0, 1},
     {-1, 0},
     {0, -1},
}};
static constexpr std::array<glm::ivec2, 8> sKingSteps    = {{
     {1, 0},
     {0, 1},
     {-1, 0},
     {0, -1},
     {1, 1},
     {-1, 1},
     {1, -1},
     {-1, -1},
}};

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
    {' ', 'p', 'n', 'b', 'r', 'q', 'k', ' ', ' ', 'P', 'N', 'B', 'R', 'Q', 'K'}};
  return sSymbols[pc];
}

Position::Position()
{
  clear();
  // Black pieces.
  put(0, Piece::B_ROK)
    .put(1, Piece::B_HRS)
    .put(2, Piece::B_BSH)
    .put(3, Piece::B_QEN)
    .put(4, Piece::B_KNG)
    .put(5, Piece::B_BSH)
    .put(6, Piece::B_HRS)
    .put(7, Piece::B_ROK);
  // Black pawns
  for (int i = 8; i < 16; ++i) {
    put(i, Piece::B_PWN);
  }
  // White pawns
  for (int i = 48; i < 56; ++i) {
    put(i, Piece::W_PWN);
  }
  // White pieces
  put(56, Piece::W_ROK)
    .put(57, Piece::W_HRS)
    .put(58, Piece::W_BSH)
    .put(59, Piece::W_QEN)
    .put(60, Piece::W_KNG)
    .put(61, Piece::W_BSH)
    .put(62, Piece::W_HRS)
    .put(63, Piece::W_ROK);
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
  BitBoard mask = BitBoard(1) << pos;
  mBitBoards[old] &= ~mask;
  mBitBoards[pc] |= mask;
  mHash ^= zobristTable()[old * 64 + pos] ^ zobristTable()[pc * 64 + pos];
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

static inline bool isOnBoard(glm::ivec2 pos)
{
  return pos[0] > -1 && pos[1] > -1 && pos[0] < 8 && pos[1] < 8;
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
      b.put(pos, B_PWN);
      shift();
      break;
    case 'n':
      b.put(pos, B_HRS);
      shift();
      break;
    case 'b':
      b.put(pos, B_BSH);
      shift();
      break;
    case 'r':
      b.put(pos, B_ROK);
      shift();
      break;
    case 'q':
      b.put(pos, B_QEN);
      shift();
      break;
    case 'k':
      b.put(pos, B_KNG);
      shift();
      break;
    case 'P':
      b.put(pos, W_PWN);
      shift();
      break;
    case 'N':
      b.put(pos, W_HRS);
      shift();
      break;
    case 'B':
      b.put(pos, W_BSH);
      shift();
      break;
    case 'R':
      b.put(pos, W_ROK);
      shift();
      break;
    case 'Q':
      b.put(pos, W_QEN);
      shift();
      break;
    case 'K':
      b.put(pos, W_KNG);
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
  for (auto it = castling.first; it != castling.second; ++it) {
    char c     = *it;
    auto match = std::find_if(sCastlingPos.begin(), sCastlingPos.end(), [c](auto tup) {
      return std::get<0>(tup) == c;
    });
    rights     = Castle(rights | std::get<1>(*match));
  }
}

static void parseEnpassant(const SubMatch& enpassant, int& enp)
{
  if (enpassant.length() > 2 || enpassant.length() < 1) {
    throw std::logic_error("Invalid enpassant target square field in the fen string");
  }
  enp = fileToX(*enpassant.first) + 8 * rankToY(*(enpassant.first + 1));
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
    glm::ivec2 pos   = {0, 0};
    int        empty = 0;
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

}  // namespace potato

namespace std {
ostream& operator<<(ostream& os, const potato::Position& b)
{
  // Copy the symbols.
  std::string str;
  str.reserve(72);
  glm::ivec2 pos;
  for (pos.y = 0; pos.y < 8; ++pos.y) {
    for (pos.x = 0; pos.x < 8; ++pos.x) {
      str.push_back(potato::symbol(b.piece(pos)));
    }
    str.push_back('\n');
  }
  os << str;
  return os;
}

}  // namespace std
