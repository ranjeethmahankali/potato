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

Position::Position()
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

Position::Position(const Position& other)
    : mRows(other.mRows)
{
  clearEnpassant();
}

bool Position::operator==(const Position& other) const
{
  return mRows == other.mRows;
}

bool Position::operator!=(const Position& other) const
{
  return !(*this == other);
}

uint8_t& Position::piece(glm::ivec2 pos)
{
  return mPieces[pos.y * 8 + pos.x];
}

uint8_t Position::piece(glm::ivec2 pos) const
{
  return mPieces[pos.y * 8 + pos.x];
}

uint8_t* Position::ptr(glm::ivec2 pos)
{
  return &(mPieces[pos.y * 8 + pos.x]);
}

const uint8_t* Position::ptr(glm::ivec2 pos) const
{
  return &(mPieces[pos.y * 8 + pos.x]);
}

glm::ivec2 Position::first() const
{
  return next(glm::ivec2 {-1, 0});
}

glm::ivec2 Position::next(glm::ivec2 pos) const
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
      pos.x = 0;
    }
  }
  return last();
}

glm::ivec2 Position::last() const
{
  return glm::ivec2 {-1, -1};
}

Position& Position::move(glm::ivec2 from, glm::ivec2 to)
{
  piece(to) = std::exchange(piece(from), Piece::NONE);
  return *this;
}

Position& Position::setMask(glm::ivec2 pos, uint8_t mask)
{
  piece(pos) |= mask;
  return *this;
}

Position& Position::clearMask(glm::ivec2 pos, uint8_t mask)
{
  piece(pos) &= ~mask;
  return *this;
}

Position& Position::setPiece(glm::ivec2 pos, uint8_t pc)
{
  piece(pos) = pc;
  return *this;
}

Position& Position::clearEnpassant()
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

static void mvBlkPwn(const Position& b, glm::ivec2 from, std::vector<Position>& dst)
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
    to           = {from.x - 1, from.y + 1};
    uint8_t topc = b.piece(to);
    if (topc && Piece::color(topc) == Piece::WHT) {
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
    to           = {from.x + 1, from.y + 1};
    uint8_t topc = b.piece(to);
    if (topc && Piece::color(topc) == Piece::WHT) {
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

static void mvWhtPwn(const Position& b, glm::ivec2 from, std::vector<Position>& dst)
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
    to           = {from.x - 1, from.y - 1};
    uint8_t topc = b.piece(to);
    if (topc && Piece::color(topc) == Piece::BLK) {
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
    to           = {from.x + 1, from.y - 1};
    uint8_t topc = b.piece(to);
    if (topc && Piece::color(topc) == Piece::BLK) {
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

static void mvHorse(const Position& b, glm::ivec2 from, std::vector<Position>& dst)
{
  uint8_t color = Piece::color(b.piece(from));
  for (glm::ivec2 move : sHorseMoves) {
    glm::ivec2 to = from + move;
    if (isOnBoard(to) && Piece::color(b.piece(to)) != color) {
      dst.emplace_back(b).move(from, to);
    }
  }
}

static void mvSliders(const Position&                  b,
                      glm::ivec2                       from,
                      std::vector<Position>&           dst,
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

static void mvBishop(const Position& b, glm::ivec2 from, std::vector<Position>& dst)
{
  mvSliders(b, from, dst, sDiagonalDirs);
}

static void mvRook(const Position& b, glm::ivec2 from, std::vector<Position>& dst)
{
  mvSliders(b, from, dst, sAxialDirs);
}

static void mvQueen(const Position& b, glm::ivec2 from, std::vector<Position>& dst)
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
static bool isAttacked(const Position& b, glm::ivec2 pos, uint8_t color)
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

static void mvKing(const Position& b, glm::ivec2 from, std::vector<Position>& dst)
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

void Position::genMoves(std::vector<Position>& dst, uint8_t turn) const
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

Position::Iterator Position::begin()
{
  return Position::Iterator(*this);
}

Position::Iterator Position::end()
{
  return Position::Iterator(*this, last());
}

Position::ConstIterator Position::begin() const
{
  return Position::ConstIterator(*this);
}

Position::ConstIterator Position::end() const
{
  return Position::ConstIterator(*this, last());
}

void Position::clear()
{
  std::fill(mRows.begin(), mRows.end(), uint64_t(0));
}

bool Position::inCheck(uint8_t color) const
{
  auto match = std::find_if(begin(), end(), [color](uint8_t pc) {
    return Piece::color(pc) == color && Piece::type(pc) == Piece::KNG;
  });
  if (match != end()) {
    return isAttacked(*this, match.pos(), color);
  }
  return false;
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

static void parseCastlingRights(const SubMatch& castling, Position& b)
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

static void parseEnpassant(const SubMatch& enpassant, Position& b)
{
  if (enpassant.length() > 2 || enpassant.length() < 1) {
    throw std::logic_error("Invalid enpassant target square field in the fen string");
  }
  b.setMask(glm::ivec2 {fileToX(*enpassant.first), rankToY(*(enpassant.first + 1))},
            Piece::ENPASSANT);
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
  parseCastlingRights(results[3], board);
  parseEnpassant(results[4], board);
  board.mHalfMoves = std::stoi(results[5]);
  board.mFullMoves = std::stoi(results[6]);
  return board;
}

std::string Position::fen() const
{
  std::string out;
  // Placement
  return out;
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
