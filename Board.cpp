#include <Board.h>
#include <algorithm>
#include <glm/fwd.hpp>

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

uint8_t Piece::color(uint8_t piece)
{
  return piece & 0b00'11'000;
}

uint8_t Piece::type(uint8_t piece)
{
  return piece & 0b111;
}

static uint8_t flipColor(uint8_t x)
{
  return x ^ 0b11000;
}

Board::Board()
{
  static constexpr std::array<uint8_t, 8> sInitialPieces = {{Piece::ROK | Piece::CASTLE,
                                                             Piece::HRS,
                                                             Piece::BSH,
                                                             Piece::QEN,
                                                             Piece::KNG | Piece::CASTLE,
                                                             Piece::BSH,
                                                             Piece::HRS,
                                                             Piece::ROK | Piece::CASTLE}};
  std::fill(mRows.begin(), mRows.end(), Piece::NONE);
  std::transform(
    sInitialPieces.begin(), sInitialPieces.end(), mPieces.begin(), [](uint8_t pc) {
      return Piece::BLK | pc;
    });
  std::fill_n(mPieces.begin() + 8, 8, Piece::BLK | Piece::PWN);
  std::fill_n(mPieces.begin() + 48, 8, Piece::WHT | Piece::PWN);
  std::transform(
    sInitialPieces.begin(), sInitialPieces.end(), mPieces.begin() + 56, [](uint8_t pc) {
      return Piece::WHT | pc;
    });
}

Board::Board(const Board& other)
    : mRows(other.mRows)
{
  clearEnpassant();
}

uint8_t& Board::piece(glm::ivec2 pos)
{
  return mPieces[pos.y * 8 + pos.x];
}

const uint8_t& Board::piece(glm::ivec2 pos) const
{
  return mPieces[pos.y * 8 + pos.x];
}

glm::ivec2 Board::first() const
{
  return next(glm::ivec2 {-1, 0});
}

glm::ivec2 Board::next(glm::ivec2 pos) const
{
  ++pos.x;
  if (pos.x > 7) {
    ++pos.y;
    pos.x %= 8;
  }
  for (; pos.y < 8; ++pos.y) {
    if (mRows[pos.y]) {
      int offset = pos.y * 8;
      for (; pos.x < 8; ++pos.x) {
        if (mPieces[offset + pos.x]) {
          return pos;
        }
      }
    }
  }
  return last();
}

glm::ivec2 Board::last() const
{
  return glm::ivec2 {-1, -1};
}

Board& Board::move(glm::ivec2 from, glm::ivec2 to)
{
  piece(to) = std::exchange(piece(from), Piece::NONE);
  return *this;
}

Board& Board::setMask(glm::ivec2 pos, uint8_t mask)
{
  piece(pos) |= mask;
  return *this;
}

Board& Board::clearMask(glm::ivec2 pos, uint8_t mask)
{
  piece(pos) &= ~mask;
  return *this;
}

Board& Board::setPiece(glm::ivec2 pos, uint8_t pc)
{
  piece(pos) = pc;
  return *this;
}

Board& Board::clearEnpassant()
{
  static constexpr uint64_t sClear =
    0b10111111'10111111'10111111'10111111'10111111'10111111'10111111'10111111;
  std::transform(
    mRows.begin(), mRows.end(), mRows.begin(), [](uint64_t row) { return row & sClear; });
  return *this;
}

static inline bool isOnBoard(glm::ivec2 pos)
{
  return pos[0] > -1 && pos[1] > -1 && pos[0] < 8 && pos[1] < 8;
}

static void mvBlkPwn(const Board& b, glm::ivec2 from, std::vector<Board>& dst)
{
  // Go forward 1
  glm::ivec2 to = {from.x, from.y + 1};
  if (!b.piece(to)) {
    auto& b2 = dst.emplace_back(b).move(from, to);
    if (to.y == 7) {  // Handle promotion
      b2.piece(to) = Piece::BLK | Piece::QEN;
      dst.emplace_back(b2).setPiece(to, Piece::BLK | Piece::ROK);
      dst.emplace_back(b2).setPiece(to, Piece::BLK | Piece::BSH);
      dst.emplace_back(b2).setPiece(to, Piece::BLK | Piece::HRS);
    }
  }
  // Go forward 2
  if (from.y == 1) {
    to = {from.x, 3};
    if (!b.piece({from.x, 2}) && !b.piece(to)) {
      dst.emplace_back(b).move(from, to).setMask(to, Piece::ENPASSANT);
    }
  }
  // Capture left
  if (from.x > 0) {
    to = {from.x - 1, from.y + 1};
    if (b.piece(to)) {
      auto& b2 = dst.emplace_back(b).move(from, to);
      if (to.y == 7) {  // Handle promotion
        b2.piece(to) = Piece::BLK | Piece::QEN;
        dst.emplace_back(b2).setPiece(to, Piece::BLK | Piece::ROK);
        dst.emplace_back(b2).setPiece(to, Piece::BLK | Piece::BSH);
        dst.emplace_back(b2).setPiece(to, Piece::BLK | Piece::HRS);
      }
    }
  }
  // Capture right
  if (from.x < 7) {
    to = {from.x + 1, from.y + 1};
    if (b.piece(to)) {
      auto& b2 = dst.emplace_back(b).move(from, to);
      if (to.y == 7) {  // Handle promotion
        b2.piece(to) = Piece::BLK | Piece::QEN;
        dst.emplace_back(b2).setPiece(to, Piece::BLK | Piece::ROK);
        dst.emplace_back(b2).setPiece(to, Piece::BLK | Piece::BSH);
        dst.emplace_back(b2).setPiece(to, Piece::BLK | Piece::HRS);
      }
    }
  }
  if (from.y == 4) {
    // Capture enpassant pawn on left
    if (from.x > 0) {
      glm::ivec2 lt = {from.x - 1, from.y};
      if (b.piece(lt) == (Piece::WHT | Piece::PWN | Piece::ENPASSANT)) {
        dst.emplace_back(b)
          .move(from, {from.x - 1, from.y + 1})
          .setPiece(lt, Piece::NONE);
      }
    }
    // Capture enpassant pawn on right
    if (from.x < 7) {
      glm::ivec2 rt = {from.x + 1, from.y};
      if (b.piece(rt) == (Piece::WHT | Piece::PWN | Piece::ENPASSANT)) {
        dst.emplace_back(b)
          .move(from, {from.x + 1, from.y + 1})
          .setPiece(rt, Piece::NONE);
      }
    }
  }
}

static void mvWhtPwn(const Board& b, glm::ivec2 from, std::vector<Board>& dst)
{
  // Go forward 1
  glm::ivec2 to = {from.x, from.y - 1};
  if (!b.piece(to)) {
    auto& b2 = dst.emplace_back(b).move(from, to);
    if (to.y == 0) {  // Handle promotion
      b2.piece(to) = Piece::WHT | Piece::QEN;
      dst.emplace_back(b2).setPiece(to, Piece::WHT | Piece::ROK);
      dst.emplace_back(b2).setPiece(to, Piece::WHT | Piece::BSH);
      dst.emplace_back(b2).setPiece(to, Piece::WHT | Piece::HRS);
    }
  }
  // Go forward 2
  if (from.y == 6) {
    to = {from.x, 4};
    if (!b.piece({from.x, 5}) && !b.piece(to)) {
      dst.emplace_back(b).move(from, to).setMask(to, Piece::ENPASSANT);
    }
  }
  // Capture left
  if (from.x > 0) {
    to = {from.x - 1, from.y - 1};
    if (b.piece(to)) {
      auto& b2 = dst.emplace_back(b).move(from, to);
      if (to.y == 0) {  // Handle promotion
        b2.piece(to) = Piece::WHT | Piece::QEN;
        dst.emplace_back(b2).setPiece(to, Piece::WHT | Piece::ROK);
        dst.emplace_back(b2).setPiece(to, Piece::WHT | Piece::BSH);
        dst.emplace_back(b2).setPiece(to, Piece::WHT | Piece::HRS);
      }
    }
  }
  // Capture right
  if (from.x < 7) {
    to = {from.x + 1, from.y - 1};
    if (b.piece(to)) {
      auto& b2 = dst.emplace_back(b).move(from, to);
      if (to.y == 0) {  // Handle promotion
        b2.piece(to) = Piece::WHT | Piece::QEN;
        dst.emplace_back(b2).setPiece(to, Piece::WHT | Piece::ROK);
        dst.emplace_back(b2).setPiece(to, Piece::WHT | Piece::BSH);
        dst.emplace_back(b2).setPiece(to, Piece::WHT | Piece::HRS);
      }
    }
  }
  if (from.y == 3) {
    // Capture enpassant pawn on left
    if (from.x > 0) {
      glm::ivec2 lt = {from.x - 1, from.y};
      if (b.piece(lt) == (Piece::BLK | Piece::PWN | Piece::ENPASSANT)) {
        dst.emplace_back(b)
          .move(from, {from.x - 1, from.y - 1})
          .setPiece(lt, Piece::NONE);
      }
    }
    if (from.y < 7) {
      glm::ivec2 rt = {from.x + 1, from.y};
      if (b.piece(rt) == (Piece::BLK | Piece::PWN | Piece::ENPASSANT)) {
        dst.emplace_back(b)
          .move(from, {from.x + 1, from.y - 1})
          .setPiece(rt, Piece::NONE);
      }
    }
    // Capture enpassant pawn on right
  }
}

static void mvHorse(const Board& b, glm::ivec2 from, std::vector<Board>& dst)
{
  uint8_t color = Piece::color(b.piece(from));
  for (glm::ivec2 move : sHorseMoves) {
    glm::ivec2 to = from + move;
    if (isOnBoard(to) && Piece::color(b.piece(to)) != color) {
      dst.emplace_back(b).move(from, to);
    }
  }
}

static void mvSliders(const Board&                     b,
                      glm::ivec2                       from,
                      std::vector<Board>&              dst,
                      const std::array<glm::ivec2, 4>& dirs)
{
  uint8_t color = Piece::color(b.piece(from));
  for (glm::ivec2 d : dirs) {
    glm::ivec2 to = from + d;
    while (isOnBoard(to)) {
      if (Piece::color(b.piece(to)) != color) {
        dst.emplace_back(b).move(from, to).clearMask(to, Piece::CASTLE);
      }
      if (b.piece(to)) {
        break;
      }
      to += d;
    }
  }
}

static void mvBishop(const Board& b, glm::ivec2 from, std::vector<Board>& dst)
{
  mvSliders(b, from, dst, sDiagonalDirs);
}

static void mvRook(const Board& b, glm::ivec2 from, std::vector<Board>& dst)
{
  mvSliders(b, from, dst, sAxialDirs);
}

static void mvQueen(const Board& b, glm::ivec2 from, std::vector<Board>& dst)
{
  mvSliders(b, from, dst, sDiagonalDirs);
  mvSliders(b, from, dst, sAxialDirs);
}

/**
 * @brief Checks if a square is attached by the given color.
 *
 * @param b Board.
 * @param pos Position to be checked.
 * @param color The attacking color
 * @return bool
 */
static bool isAttacked(const Board& b, glm::ivec2 pos, uint8_t color)
{
  if (color != Piece::BLK && color != Piece::WHT) {  // Makes no sense.
    return false;
  }
  uint8_t pc = b.piece(pos);
  if (Piece::color(pc) == color) {  // Can't attack yourself.
    return false;
  }
  // Look for pawns
  int yStep = color == Piece::BLK ? 1 : -1;
  if (pos.x > 0) {                                      // Left
    uint8_t apc = b.piece({pos.x - 1, pos.y - yStep});  // Potential attacking piece.
    if (Piece::color(apc) == color && Piece::type(apc) == Piece::PWN) {
      return true;
    }
  }
  if (pos.x < 7) {  // Right
    uint8_t apc = b.piece({pos.x + 1, pos.y - yStep});
    if (Piece::color(apc) == color && Piece::type(apc) == Piece::PWN) {
      return true;
    }
  }
  // Enpassant
  int yRow = color == Piece::BLK ? 4 : 3;
  if (pos.y == yRow && (pc & Piece::ENPASSANT)) {
    if (pos.x > 0 && b.piece({pos.x - 1, pos.y}) == (Piece::PWN | color)) {  // Left
      return true;
    }
    if (pos.x < 7 && b.piece({pos.x + 1, pos.y}) == (Piece::PWN | color)) {  // Right
      return true;
    }
  }
  // Look for horses
  for (glm::ivec2 move : sHorseMoves) {
    glm::ivec2 p = pos + move;
    if (isOnBoard(p) && b.piece(p) == (Piece::HRS | color)) {
      return true;
    }
  }
  // March diagonals and look for bishops or queens
  for (glm::ivec2 dir : sDiagonalDirs) {
    glm::ivec2 att = pos + dir;
    while (isOnBoard(att)) {
      uint8_t apc  = b.piece(att);
      uint8_t type = Piece::type(apc);
      if (Piece::color(apc) == color && (type == Piece::BSH || type == Piece::QEN)) {
        return true;
      }
      else if (!pc) {
        break;
      }
    }
  }
  // March along rows and cols and look for rooks and queens
  for (glm::ivec2 dir : sAxialDirs) {
    glm::ivec2 att = pos + dir;
    while (isOnBoard(att)) {
      uint8_t apc  = b.piece(att);
      uint8_t type = Piece::type(apc);
      if (Piece::color(apc) == color && (type == Piece::ROK || type == Piece::QEN)) {
        return true;
      }
      else if (!pc) {
        break;
      }
    }
  }
  // Look for enemy king
  for (glm::ivec2 dir : sKingSteps) {
    glm::ivec2 apos = pos + dir;
    if (isOnBoard(apos)) {
      uint8_t apc = b.piece(apos);
      if (Piece::color(apc) == color && Piece::type(apc) == Piece::KNG) {
        return true;
      }
    }
  }
  return false;
}

static void mvKing(const Board& b, glm::ivec2 from, std::vector<Board>& dst)
{
  uint8_t pc    = b.piece(from);
  uint8_t color = Piece::color(pc);
  for (glm::ivec2 step : sKingSteps) {
    glm::ivec2 to = from + step;
    if (isOnBoard(to) && Piece::color(b.piece(to)) != color) {
      dst.emplace_back(b).move(from, to).clearMask(to, Piece::CASTLE);
    }
  }
  // Castling.
  int homeRow = color == Piece::BLK ? 0 : 7;
  if (from == glm::ivec2 {4, homeRow} && (pc & Piece::CASTLE)) {  // King is ready.
    // Try castle long.
    if (b.piece(glm::ivec2 {0, homeRow}) ==
        (color | Piece::ROK | Piece::CASTLE)) {  // Rook is ready
      std::array<glm::ivec2, 3> noCheck = {{{2, homeRow}, {3, homeRow}, {4, homeRow}}};
      std::array<glm::ivec2, 3> clear   = {{{1, homeRow}, {2, homeRow}, {3, homeRow}}};
      if (std::all_of(
            noCheck.begin(),
            noCheck.end(),
            [&](glm::ivec2 p) { return !isAttacked(b, p, flipColor(color)); }) &&
          std::all_of(clear.begin(), clear.end(), [&](glm::ivec2 p) {
            return !b.piece(p);
          })) {  // Path is clear
        glm::ivec2 kpos = glm::ivec2 {2, homeRow};
        glm::ivec2 rpos = {3, homeRow};
        dst.emplace_back(b)
          .move(from, kpos)
          .move(glm::ivec2 {0, homeRow}, rpos)
          .clearMask(kpos, Piece::CASTLE)
          .clearMask(rpos, Piece::CASTLE);
      }
    }
    // Try castle short.
    if (b.piece(glm::ivec2 {7, homeRow}) ==
        (color | Piece::ROK | Piece::CASTLE)) {  // Rook is ready
      std::array<glm::ivec2, 2> clear   = {{{5, homeRow}, {6, homeRow}}};
      std::array<glm::ivec2, 3> noCheck = {{{4, homeRow}, {5, homeRow}, {6, homeRow}}};
      if (std::all_of(
            noCheck.begin(),
            noCheck.end(),
            [&](glm::ivec2 p) { return !isAttacked(b, p, flipColor(color)); }) &&
          std::all_of(clear.begin(), clear.end(), [&](glm::ivec2 p) {
            return !b.piece(p);
          })) {  // Path is clear
        glm::ivec2 kpos = glm::ivec2 {6, homeRow};
        glm::ivec2 rpos = {5, homeRow};
        dst.emplace_back(b)
          .move(from, kpos)
          .move(glm::ivec2 {7, homeRow}, rpos)
          .clearMask(kpos, Piece::CASTLE)
          .clearMask(rpos, Piece::CASTLE);
      }
    }
  }
}

void Board::genMoves(std::vector<Board>& dst, uint8_t turn) const
{
  static constexpr std::array<glm::ivec2, 2> sForward    = {{{0, 1}, {0, -1}}};
  static constexpr std::array<int, 2>        sPwnHomeRow = {{1, 6}};
  // Iterate over the pieces.
  glm::ivec2 pos = first();
  while (pos != last()) {
    uint8_t pc       = piece(pos);
    uint8_t color    = Piece::color(pc);
    int     colorIdx = int(color) >> 4;
    uint8_t type     = Piece::type(pc);
    if (color == turn) {
      switch (type) {
      case Piece::PWN:
        if (color == Piece::BLK) {
          mvBlkPwn(*this, pos, dst);
        }
        else if (color == Piece::WHT) {
          mvWhtPwn(*this, pos, dst);
        }
        break;
      case Piece::HRS:
        mvHorse(*this, pos, dst);
        break;
      case Piece::BSH:
        mvBishop(*this, pos, dst);
        break;
      case Piece::ROK:
        mvRook(*this, pos, dst);
        break;
      case Piece::QEN:
        mvQueen(*this, pos, dst);
        break;
      case Piece::KNG:
        break;
      }
    }
    pos = next(pos);
  }
}

Board::Iterator Board::begin()
{
  return Board::Iterator(*this);
}

Board::Iterator Board::end()
{
  return Board::Iterator(*this, last());
}

Board::ConstIterator Board::begin() const
{
  return Board::ConstIterator(*this);
}

Board::ConstIterator Board::end() const
{
  return Board::ConstIterator(*this, last());
}

}  // namespace potato

namespace std {
ostream& operator<<(ostream& os, const potato::Board& b)
{
  static constexpr std::array<char, 256> sAsciiTable = {{
    95, 95, 95, 95, 95, 95, 95, 95, 95, 112, 98, 110, 114, 113, 107, 95,
    95, 80, 66, 78, 82, 81, 75, 95, 95, 95,  95, 95,  95,  95,  95,  95,
    95, 95, 95, 95, 95, 95, 95, 95, 95, 112, 98, 110, 114, 113, 107, 95,
    95, 80, 66, 78, 82, 81, 75, 95, 95, 95,  95, 95,  95,  95,  95,  95,
    95, 95, 95, 95, 95, 95, 95, 95, 95, 112, 98, 110, 114, 113, 107, 95,
    95, 80, 66, 78, 82, 81, 75, 95, 95, 95,  95, 95,  95,  95,  95,  95,
    95, 95, 95, 95, 95, 95, 95, 95, 95, 112, 98, 110, 114, 113, 107, 95,
    95, 80, 66, 78, 82, 81, 75, 95, 95, 95,  95, 95,  95,  95,  95,  95,
    95, 95, 95, 95, 95, 95, 95, 95, 95, 112, 98, 110, 114, 113, 107, 95,
    95, 80, 66, 78, 82, 81, 75, 95, 95, 95,  95, 95,  95,  95,  95,  95,
    95, 95, 95, 95, 95, 95, 95, 95, 95, 112, 98, 110, 114, 113, 107, 95,
    95, 80, 66, 78, 82, 81, 75, 95, 95, 95,  95, 95,  95,  95,  95,  95,
    95, 95, 95, 95, 95, 95, 95, 95, 95, 112, 98, 110, 114, 113, 107, 95,
    95, 80, 66, 78, 82, 81, 75, 95, 95, 95,  95, 95,  95,  95,  95,  95,
    95, 95, 95, 95, 95, 95, 95, 95, 95, 112, 98, 110, 114, 113, 107, 95,
    95, 80, 66, 78, 82, 81, 75, 95, 95, 95,  95, 95,  95,  95,  95,  95,
  }};
  // Copy the symbols.
  std::string str;
  str.reserve(72);
  glm::ivec2 pos;
  for (pos.y = 0; pos.y < 8; ++pos.y) {
    for (pos.x = 0; pos.x < 8; ++pos.x) {
      str.push_back(sAsciiTable[b.piece(pos)]);
    }
    str.push_back('\n');
  }
  os << str;
  return os;
}
}  // namespace std
