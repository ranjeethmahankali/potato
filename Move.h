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
  SILENT  = 0,
  CAPTURE = 1,   // r
  MV_KNG,        // c
  MV_ROK,        // c
  PUSH,          // r
  DBL_PUSH,      // r
  ENPASSANT,     // r
  PRM_HRS,       // r
  PRM_BSH,       // r
  PRM_ROK,       // r
  PRM_QEN,       // r
  PRC_HRS,       // r
  PRC_BSH,       // r
  PRC_ROK,       // r
  PRC_QEN,       // r
  CASTLE_SHORT,  // c
  CASTLE_LONG,   // c
};

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
  MoveType mType = SILENT;
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
  void append(MoveType type, int from, int to);
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
