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
  if constexpr (Col) {
    return dir;
  }
  else {
    return Direction(-dir);
  }
}

template<Color col, int rank>
static constexpr int RelativeRank = col == Color::WHT ? (7 - rank) : rank;

template<Color Player>
struct MvPiece
{
  int mFrom;
  int mTo;

  void commit(Position& p)
  {
    p.history().push({.mPiece = p.piece(mTo)});
    p.move(mFrom, mTo);
  }
  void revert(Position& p) { p.move(mTo, mFrom).put(mTo, p.history().pop().mPiece); }
};

template<Color Player>
struct MvEnpassant
{
  int       mFrom;
  Direction mSide;

  void commit(Position& p)
  {
    int target = mFrom + relativeDir<Player>(mSide);
    p.move(mFrom, target + RelativeDir<N, Player>).remove(target);
  }
  void revert(Position& p)
  {
    static constexpr Color Enemy  = Color(~Player);
    int                    target = mFrom + relativeDir<Player>(mSide);
    p.move(target + RelativeDir<N, Player>, mFrom)
      .put(target, Piece(uint8_t(Enemy) | PieceType::PWN));
  }
};

template<Color Player>
struct MvDoublePush
{
  int mFrom;

  void commit(Position& p)
  {
    p.move(mFrom, mFrom + 2 * RelativeDir<N, Player>);
    p.setEnpassantSq(mFrom + 2 * RelativeDir<N, Player>);
  }
  void revert(Position& p) { p.move(mFrom + 2 * RelativeDir<N, Player>, mFrom); }
};

template<Color Player>
struct MvPromote
{
  int   mFile;
  Piece mPromoted;

  void commit(Position& p)
  {
    p.remove(glm::ivec2 {mFile, RelativeRank<Player, 6>})
      .put(glm::ivec2 {mFile, RelativeRank<Player, 7>}, mPromoted);
  }
  void revert(Position& p)
  {
    p.remove(glm::ivec2 {mFile, RelativeRank<Player, 7>})
      .put(glm::ivec2 {mFile, RelativeRank<Player, 6>},
           Piece(uint8_t(Player) | PieceType::PWN));
  }
};

template<Color Player>
struct MvCapturePromote
{
  int       mFile;
  Direction mSide;
  Piece     mPromoted;

  void commit(Position& p)
  {
    glm::ivec2 dst = {mFile + relativeDir<Player>(mSide), RelativeRank<Player, 7>};
    p.history().push({.mPiece = p.piece(dst)});
    p.remove(glm::ivec2 {mFile, RelativeRank<Player, 6>}).put(dst, mPromoted);
  }
  void revert(Position& p)
  {
    p.put(glm::ivec2 {mFile + relativeDir<Player>(mSide), RelativeRank<Player, 7>},
          p.history().pop().mPiece)
      .put(glm::ivec2 {mFile, RelativeRank<Player, 6>},
           Piece(uint8_t(Player) | PieceType::PWN));
  }
};

template<Color Player>
struct MvCastleShort
{
  static constexpr int Rank = RelativeRank<Player, 0>;
  void commit(Position& p) { p.move({4, Rank}, {6, Rank}).move({7, Rank}, {5, Rank}); }
  void revert(Position& p) { p.move({5, Rank}, {7, Rank}).move({6, Rank}, {4, Rank}); }
};

template<Color Player>
struct MvCastleLong
{
  static constexpr int Rank = RelativeRank<Player, 0>;
  void commit(Position& p) { p.move({4, Rank}, {2, Rank}).move({0, Rank}, {3, Rank}); }
  void revert(Position& p) { p.move({2, Rank}, {4, Rank}).move({3, Rank}, {0, Rank}); }
};

template<template<Color> typename... MoveTypes>
struct TMoveVariant
{
  using Type = std::variant<MoveTypes<Color::WHT>..., MoveTypes<Color::BLK>...>;
};

struct Move  // Wraps all moves in a variant.
{
private:
  using VariantType = typename TMoveVariant<MvPiece,
                                            MvDoublePush,
                                            MvEnpassant,
                                            MvPromote,
                                            MvCapturePromote,
                                            MvCastleShort,
                                            MvCastleLong>::Type;
  VariantType mVar;

public:
  Move() = default;
  template<typename TMove>
  explicit Move(const TMove m)
      : mVar(m)
  {}

  void commit(Position& p);
  void revert(Position& p);
};

struct MoveList
{
  MoveList();
  const Move* begin() const;
  const Move* end() const;
  size_t      size() const;
  void        clear();

private:
  static constexpr size_t MaxMoves = 256;
  std::array<Move, 256>   mBuf;
  Move*                   mEnd;

public:
  template<typename TMove>
  void operator+=(const TMove& mv)
  {
    *(mEnd++) = Move(mv);
  }
};

static constexpr BitBoard NotAFile = ~File[0];
static constexpr BitBoard NotHFile = ~File[7];

template<Direction Dir>
constexpr BitBoard shift(BitBoard b)
{
  if constexpr (Dir == SW) {
    return b >> 7 & NotAFile;
  }
  else if constexpr (Dir == S) {
    return b >> 8;
  }
  else if constexpr (Dir == SE) {
    return b >> 9 & NotHFile;
  }
  else if constexpr (Dir == E) {
    return b >> 1 & NotHFile;
  }
  else if constexpr (Dir == NE) {
    return b << 7 & NotHFile;
  }
  else if constexpr (Dir == N) {
    return b << 8;
  }
  else if constexpr (Dir == NW) {
    return b << 9 & NotAFile;
  }
  else if constexpr (Dir == W) {
    return b << 1 & NotAFile;
  }
  else {
    return b;
  }
}

template<Direction Dir>
constexpr BitBoard dblshift(BitBoard b)
{
  return shift<Dir>(shift<Dir>(b));
}

int      pop(BitBoard& b);
int      lsb(BitBoard b);
BitBoard bishopMoves(int sq, BitBoard blockers);
BitBoard rookMoves(int sq, BitBoard blockers);
BitBoard queenMoves(int sq, BitBoard blockers);

template<Color Player>
Piece makePiece(PieceType type)
{
  return Piece(uint8_t(Player) | type);
}

template<Color Player, PieceType... Types>
BitBoard getBoard(const Position& p)
{
  return (p.board(makePiece<Player>(Types)) | ...);
}

template<Color Player>
BitBoard getAllBoards(const Position& p)
{
  return getBoard<Player, PWN, HRS, BSH, ROK, QEN, KNG>(p);
}

template<Color Player>
BitBoard pawnCaptures(int pos)
{
  if constexpr (Player == WHT) {
    return WhitePawnCaptures[pos];
  }
  else {
    return BlackPawnCaptures[pos];
  }
}

template<Color Player>
BitBoard pawnCaptures(BitBoard b)
{
  return shift<RelativeDir<NW, Player>>(b) | shift<RelativeDir<NE, Player>>(b);
}

template<Color Player>
void generateMoves(const Position& p, MoveList& moves)
{
  static constexpr Color Enemy        = Player == BLK ? WHT : BLK;
  BitBoard               self         = getAllBoards<Player>(p);
  BitBoard               notself      = ~self;
  BitBoard               enemy        = getAllBoards<Enemy>(p);
  BitBoard               notenemy     = ~enemy;
  BitBoard               all          = self | enemy;
  BitBoard               empty        = ~all;
  BitBoard               ourKing      = getBoard<Player, KNG>(p);
  BitBoard               otherKing    = getBoard<Enemy, KNG>(p);
  int                    kingPos      = lsb(ourKing);
  int                    otherKingPos = lsb(otherKing);
  BitBoard               unsafe       = 0;
  {  // Find all unsafe squares.
    unsafe =
      pawnCaptures<Enemy>(getBoard<Enemy, PWN>(p)) | (KingMoves[otherKingPos] & empty);
    auto sliders = getBoard<Enemy, BSH, QEN>(p);
    while (sliders) {
      unsafe |= bishopMoves(pop(sliders), all);
    }
    sliders = getBoard<Enemy, ROK, QEN>(p);
    while (sliders) {
      unsafe |= rookMoves(pop(sliders), all);
    }
    auto kmoves = KingMoves[kingPos] & (~(unsafe | self));
    while (kmoves) {
      moves += MvPiece<Player> {kingPos, pop(kmoves)};
    }
  }
  BitBoard pins     = 0;
  BitBoard checkers = 0;
  {
    checkers |= (KnightMoves[ourKing] & getBoard<Enemy, HRS>(p)) |
                (pawnCaptures<Player>(kingPos) & getBoard<Enemy, PWN>(p));
    // Look for checks and pins from sliders.
    auto diags  = bishopMoves(kingPos, enemy) & getBoard<Enemy, BSH, QEN>(p);
    auto orthos = rookMoves(kingPos, enemy) & getBoard<Enemy, ROK, QEN>(p);
    // Slideres that are lined up with the king.
    auto sliders = diags | orthos;
    while (sliders) {
      int  spos = pop(sliders);
      auto line = Between[spos][kingPos];
      switch (std::popcount(line & self)) {
      case 0:  // The king is in check
        checkers |= OneHot[spos];
        break;
      case 1:  // The pin line
        pins |= line;
        break;
      case 2:  // Not in check, not pinned.
        break;
      }
      if (OneHot[spos] & diags) {  // This is a diagonal slider.
        unsafe |= bishopMoves(spos, all);
      }
      else if (OneHot[spos] & orthos) {
        unsafe |= rookMoves(spos, all);
      }
    }
  }
  switch (std::popcount(checkers)) {
  case 0:  // Do nothing.
    break;
  case 1: {
    // Can block, or capture the checker. We already generated all possible king moves.
    // Try to capture.
    auto cpos      = lsb(checkers);
    auto line      = Between[cpos][kingPos];
    auto attackers = getBoard<Player, PWN>(p);
    // NW pawn captures.
    auto captures = shift<RelativeDir<NW, Player>>(attackers) & checkers;
    while (captures) {
      moves += MvPiece<Player> {cpos - RelativeDir<NW, Player>, cpos};
    }
    // NE pawn captures.
    captures = shift<RelativeDir<NE, Player>>(attackers) & checkers;
    while (captures) {
      moves += MvPiece<Player> {cpos - RelativeDir<NE, Player>, cpos};
    }
    // Enpassant captures.
    if (p.enpassantSq() == cpos && p.piece(cpos) == makePiece<Player>(PWN)) {
      attackers = shift<RelativeDir<E, Player>>(checkers) & getBoard<Player, PWN>(p);
      while (attackers) {
        moves += MvEnpassant<Player> {pop(attackers), RelativeDir<W, Player>};
      }
      attackers = shift<RelativeDir<W, Player>>(checkers) & getBoard<Player, PWN>(p);
      while (attackers) {
        moves += MvEnpassant<Player> {pop(attackers), RelativeDir<E, Player>};
      }
    }
    // Single push pawn blocks.
    auto blocked = shift<RelativeDir<N, Player>>(getBoard<Player, PWN>(p)) & line;
    if (blocked) {
      int bpos = lsb(blocked);
      moves += MvPiece<Player> {bpos - RelativeDir<N, Player>, bpos};
    }
    // Double push pawn blocks.
    blocked = dblshift<RelativeDir<N, Player>>(getBoard<Player, PWN>(p)) & line;
    if (blocked) {
      moves += MvDoublePush<Player> {pop(blocked) - 2 * RelativeDir<N, Player>};
    }
    // Knight captures and blocks.
    attackers = getBoard<Player, HRS>(p);
    while (attackers) {
      int hpos = pop(attackers);
      if (KnightMoves[hpos] & checkers) {
        moves += MvPiece<Player> {hpos, cpos};
      }
      blocked = KnightMoves[hpos] & line;
      if (blocked) {
        moves += MvPiece<Player> {hpos, pop(blocked)};
      }
    }
    // Diag slider captures and blocks.
    attackers = getBoard<Player, BSH, QEN>();
    while (attackers) {
      int  dpos   = pop(attackers);
      auto dmoves = bishopMoves(dpos, all);
      if (dmoves & checkers) {
        moves += MvPiece<Player> {dpos, cpos};
      }
      blocked = dmoves & line;
      if (blocked) {
        moves += MvPiece<Player> {dpos, pop(blocked)};
      }
    }
    // Ortho slider captures
    attackers = getBoard<Player, ROK, QEN>();
    while (attackers) {
      int  opos   = pop(attackers);
      auto omoves = rookMoves(opos, all);
      if (omoves & checkers) {
        moves += MvPiece<Player> {opos, cpos};
      }
      blocked = omoves & line;
      if (blocked) {
        moves += MvPiece<Player> {opos, pop(blocked)};
      }
    }
    // Generated all the moves to get out of check.
    // No more legal moves.
    return;
  }
  case 2:
    // The king must move to a safe square.
    // We already generated all possible king moves, so we stop looking for other moves.
    return;
  }
  {  // Pawn single push
    auto nopins = ~pins;
    auto pcs    = getBoard<Player, PWN>(p);
    auto pmoves =
      // pinned
      (shift<RelativeDir<N, Player>>(pcs & pins) & empty & pins) |
      // unpinned
      (shift<RelativeDir<N, Player>>(pcs & nopins) & empty);
    while (pmoves) {
      int pos = pop(pmoves);
      moves += MvPiece<Player> {pos - RelativeDir<N, Player>, pos};
    }
  }
  // TODO: Incomplete.
}

}  // namespace potato
