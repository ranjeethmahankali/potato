#pragma once

#include <Position.h>
#include <Tables.h>
#include <stdint.h>
#include <bit>
#include <variant>

namespace potato {

enum Direction : int
{
  SW = -7,
  S  = -8,
  SE = -9,
  E  = -1,
  NE = 7,
  N  = 8,
  NW = 9,
  W  = 1,
};

template<Direction dir, Color col>
static constexpr Direction RelativeDir = col == Color::WHT ? Direction(-dir) : dir;

template<Color Col>
inline Direction relativeDir(Direction dir)
{
  if constexpr (Col == BLK) {
    return dir;
  }
  else {
    return Direction(-dir);
  }
}

template<Color col, int rank>
static constexpr int RelativeRank = col == Color::WHT ? (7 - rank) : rank;

enum MoveType : uint8_t
{
  MV_KNG       = 0,   // c
  MV_ROK       = 1,   // c
  PUSH         = 2,   // r
  DBL_PUSH     = 3,   // r
  ENPASSANT    = 4,   // r
  PRM_HRS      = 5,   // r
  PRM_BSH      = 6,   // r
  PRM_ROK      = 7,   // r
  PRM_QEN      = 8,   // r
  PRC_HRS      = 9,   // r
  PRC_BSH      = 10,  // r
  PRC_ROK      = 11,  // r
  PRC_QEN      = 12,  // r
  CASTLE_SHORT = 13,  // c
  CASTLE_LONG  = 14,  // c
  OTHER        = 15,  // Move with knight, bishop or queen.
  CAPTURE      = 16,  // This is used as a mask on other moves.
  // Eventhough enpassant is a capture, this mask is not used for that, as it is handled
  // seprately.
};

constexpr inline MoveType operator|(MoveType a, MoveType b)
{
  return MoveType(uint8_t(a) | b);
}

constexpr inline MoveType operator&(MoveType a, MoveType b)
{
  return MoveType(uint8_t(a) & b);
}

struct Move
{
  Move() = default;
  Move(MoveType type, int from, int to);
  void     assign(MoveType type, int from, int to);
  MoveType type() const;
  int      from() const;
  int      to() const;
  void     commit(Position& p) const;
  void     revert(Position& p) const;

private:
  MoveType mType = OTHER;
  uint8_t  mFrom = UINT8_MAX;
  uint8_t  mTo   = UINT8_MAX;
};

struct MoveList
{
  MoveList();
  MoveList(const MoveList&);
  MoveList(MoveList&&);
  const MoveList& operator=(const MoveList&);
  const MoveList& operator=(MoveList&&);
  const Move*     begin() const;
  const Move*     end() const;
  size_t          size() const;
  void            clear();
  const Move&     operator[](size_t i) const;

private:
  static constexpr size_t MaxMoves = 256;
  std::array<Move, 256>   mBuf;
  Move*                   mEnd;

public:
  void append(MoveType type, int from, int to, bool isCapture = false);
};

int      pop(BitBoard& b);
int      lsb(BitBoard b);
BitBoard bishopMoves(int sq, BitBoard blockers);
BitBoard rookMoves(int sq, BitBoard blockers);
BitBoard queenMoves(int sq, BitBoard blockers);
void     generateMoves(const Position& p, MoveList& moves);
void     perft(const Position& p, int depth);

template<Color Player, PieceType... Types>
BitBoard getBoard(const Position& p)
{
  return (p.board(Player | Types) | ...);
}

template<Color Player>
BitBoard getAllBoards(const Position& p)
{
  return getBoard<Player, PWN, HRS, BSH, ROK, QEN, KNG>(p);
}

}  // namespace potato

namespace std {

std::ostream& operator<<(std::ostream& os, const potato::Move& m);

}  // namespace std
