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

  void commit(Position& p) const

  {
    p.history().push({.mPiece = p.piece(mTo)});
    p.move(mFrom, mTo);
  }
  void revert(Position& p) const
  {
    p.move(mTo, mFrom).put(mTo, p.history().pop().mPiece);
  }
};

template<Color Player>
struct MvEnpassant
{
  int       mFrom;
  Direction mSide;

  int  target() const { return mFrom + relativeDir<Player>(mSide); }
  int  dest() const { return target() + RelativeDir<N, Player>; }
  void commit(Position& p) const { p.move(mFrom, dest()).remove(target()); }
  void revert(Position& p) const
  {
    static constexpr Color Enemy = Color(~Player);
    p.move(dest(), mFrom).put(target(), Piece(uint8_t(Enemy) | PieceType::PWN));
  }
};

template<Color Player>
struct MvDoublePush
{
  int mFrom;

  int dest() const { return mFrom + 2 * RelativeDir<N, Player>; }

  void commit(Position& p) const
  {
    int d = dest();
    p.move(mFrom, d);
    p.setEnpassantSq(d);
  }
  void revert(Position& p) const { p.move(dest(), mFrom); }
};

template<Color Player>
struct MvPromote
{
  uint8_t mFile;
  Piece   mPromoted;

  void commit(Position& p) const

  {
    p.remove(glm::ivec2 {int(mFile), RelativeRank<Player, 6>})
      .put(glm::ivec2 {int(mFile), RelativeRank<Player, 7>}, mPromoted);
  }
  void revert(Position& p) const
  {
    p.remove(glm::ivec2 {int(mFile), RelativeRank<Player, 7>})
      .put(glm::ivec2 {int(mFile), RelativeRank<Player, 6>},
           Piece(uint8_t(Player) | PieceType::PWN));
  }
};

template<Color Player>
struct MvCapturePromote
{
  uint8_t   mFile;
  Direction mSide;
  Piece     mPromoted;

  void commit(Position& p) const

  {
    glm::ivec2 dst = {int(mFile) + relativeDir<Player>(mSide), RelativeRank<Player, 7>};
    p.history().push({.mPiece = p.piece(dst)});
    p.remove(glm::ivec2 {int(mFile), RelativeRank<Player, 6>}).put(dst, mPromoted);
  }
  void revert(Position& p) const
  {
    p.put(glm::ivec2 {int(mFile) + relativeDir<Player>(mSide), RelativeRank<Player, 7>},
          p.history().pop().mPiece)
      .put(glm::ivec2 {int(mFile), RelativeRank<Player, 6>},
           Piece(uint8_t(Player) | PieceType::PWN));
  }
};

template<Color Player>
struct MvCastleShort
{
  static constexpr int Rank = RelativeRank<Player, 0>;
  void                 commit(Position& p) const
  {
    p.move({4, Rank}, {6, Rank}).move({7, Rank}, {5, Rank});
  }
  void revert(Position& p) const
  {
    p.move({5, Rank}, {7, Rank}).move({6, Rank}, {4, Rank});
  }
};

template<Color Player>
struct MvCastleLong
{
  static constexpr int Rank = RelativeRank<Player, 0>;
  void                 commit(Position& p) const
  {
    p.move({4, Rank}, {2, Rank}).move({0, Rank}, {3, Rank});
  }
  void revert(Position& p) const
  {
    p.move({2, Rank}, {4, Rank}).move({3, Rank}, {0, Rank});
  }
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

  const VariantType& value() const;
  void               commit(Position& p) const;
  void               revert(Position& p) const;
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
BitBoard pawnCapturesFromPos(int pos)
{
  if constexpr (Player == WHT) {
    return WhitePawnCaptures[pos];
  }
  else {
    return BlackPawnCaptures[pos];
  }
}

template<Direction Dir, Color Player>
BitBoard pawnCaptures(BitBoard b)
{
  if constexpr (Dir == W || Dir == NW) {
    return shift<RelativeDir<NW, Player>>(b);
  }
  else if constexpr (Dir == E || Dir == NE) {
    return shift<RelativeDir<NE, Player>>(b);
  }
  else {
    return 0;
  }
}

template<Color Player>
void generateMoves(const Position& p, MoveList& moves)
{
  static constexpr Direction Up            = RelativeDir<N, Player>;
  static constexpr BitBoard  HomePawnRank  = Rank[RelativeRank<Player, 1> * 8];
  static constexpr BitBoard  PromotionRank = Rank[RelativeRank<Player, 6> * 8];
  static constexpr Color     Enemy         = Player == BLK ? WHT : BLK;
  BitBoard                   self          = getAllBoards<Player>(p);
  BitBoard                   notself       = ~self;
  BitBoard                   enemy         = getAllBoards<Enemy>(p);
  BitBoard                   notenemy      = ~enemy;
  BitBoard                   all           = self | enemy;
  BitBoard                   empty         = ~all;
  BitBoard                   ourKing       = getBoard<Player, KNG>(p);
  BitBoard                   otherKing     = getBoard<Enemy, KNG>(p);
  int                        kingPos       = lsb(ourKing);
  int                        otherKingPos  = lsb(otherKing);
  BitBoard                   unsafe        = 0;
  {  // Find all unsafe squares.
    BitBoard pcs = getBoard<Enemy, PWN>(p);
    unsafe       = pawnCaptures<NE, Enemy>(pcs) | pawnCaptures<NW, Enemy>(pcs) |
             (KingMoves[otherKingPos] & empty);
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
    checkers |= (KnightMoves[kingPos] & getBoard<Enemy, HRS>(p)) |
                (pawnCapturesFromPos<Player>(kingPos) & getBoard<Enemy, PWN>(p));
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
    auto blocked = shift<Up>(getBoard<Player, PWN>(p)) & line;
    if (blocked) {
      int bpos = lsb(blocked);
      moves += MvPiece<Player> {bpos - Up, bpos};
    }
    // Double push pawn blocks.
    blocked =
      shift<Up>(shift<Up>(HomePawnRank & (getBoard<Player, PWN>(p))) & empty) & line;
    if (blocked) {
      moves += MvDoublePush<Player> {pop(blocked) - 2 * Up};
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
    attackers = getBoard<Player, BSH, QEN>(p);
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
    attackers = getBoard<Player, ROK, QEN>(p);
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
    // We already generated all possible king moves, so we stop looking for other mvoes.
    return;
  }
  {  // Pawn single push
    auto nopins = ~pins;
    auto pcs    = getBoard<Player, PWN>(p);
    auto pmoves =
      // pinned
      (shift<Up>(pcs & pins) & empty & pins) |
      // unpinned
      (shift<Up>(pcs & nopins) & empty);
    while (pmoves) {
      int pos = pop(pmoves);
      moves += MvPiece<Player> {pos - Up, pos};
    }
    // Pawn double push
    pcs &= HomePawnRank;
    pmoves =
      // unpinned
      (shift<Up>(shift<Up>(pcs & nopins) & empty) & empty) |
      // pinned
      (shift<Up>(shift<Up>(pcs & pins) & empty & pins) & empty & pins);
    while (pmoves) {
      int pos = pop(pmoves);
      moves += MvDoublePush<Player> {pos - 2 * Up};
    }
    // Pawn captures
    pcs    = getBoard<Player, PWN>(p);
    pmoves = pawnCaptures<NE, Player>(pcs) & enemy;  // To east.
    while (pmoves) {
      int pos = pop(pmoves);
      moves += MvPiece<Player> {pos - RelativeDir<NE, Player>, pos};
    }
    pmoves = pawnCaptures<NW, Player>(pcs) & enemy;  // To west.
    while (pmoves) {
      int pos = pop(pmoves);
      moves += MvPiece<Player> {pos - RelativeDir<NW, Player>, pos};
    }
    // Pawn promotions.
    pcs    = getBoard<Player, PWN>(p) & PromotionRank;
    pmoves = shift<Up>(pcs) & empty;
    while (pmoves) {
      uint8_t file = uint8_t(pop(pmoves) % 8);
      moves += MvPromote<Player> {file, makePiece<Player>(QEN)};
      moves += MvPromote<Player> {file, makePiece<Player>(ROK)};
      moves += MvPromote<Player> {file, makePiece<Player>(BSH)};
      moves += MvPromote<Player> {file, makePiece<Player>(HRS)};
    }
    // Pawn capture promotions.
    pcs    = getBoard<Player, PWN>(p) & PromotionRank;
    pmoves = shift<RelativeDir<NE, Player>>(pcs) & enemy;
    while (pmoves) {
      uint8_t file = uint8_t(pop(pmoves) % 8);
      moves +=
        MvCapturePromote<Player> {file, RelativeDir<E, Player>, makePiece<Player>(QEN)};
      moves +=
        MvCapturePromote<Player> {file, RelativeDir<E, Player>, makePiece<Player>(ROK)};
      moves +=
        MvCapturePromote<Player> {file, RelativeDir<E, Player>, makePiece<Player>(BSH)};
      moves +=
        MvCapturePromote<Player> {file, RelativeDir<E, Player>, makePiece<Player>(HRS)};
    }
    pcs    = getBoard<Player, PWN>(p) & PromotionRank;
    pmoves = shift<RelativeDir<NW, Player>>(pcs) & enemy;
    while (pmoves) {
      uint8_t file = uint8_t(pop(pmoves) % 8);
      moves +=
        MvCapturePromote<Player> {file, RelativeDir<W, Player>, makePiece<Player>(QEN)};
      moves +=
        MvCapturePromote<Player> {file, RelativeDir<W, Player>, makePiece<Player>(ROK)};
      moves +=
        MvCapturePromote<Player> {file, RelativeDir<W, Player>, makePiece<Player>(BSH)};
      moves +=
        MvCapturePromote<Player> {file, RelativeDir<W, Player>, makePiece<Player>(HRS)};
    }
    // Enpassant
    if (p.enpassantSq() != -1) {
      pcs    = getBoard<Player, PWN>(p);
      pmoves = shift<RelativeDir<E, Player>>(OneHot[p.enpassantSq()]) & pcs;
      moves += MvEnpassant<Player> {lsb(pmoves), RelativeDir<W, Player>};
      pmoves = shift<RelativeDir<W, Player>>(OneHot[p.enpassantSq()]) & pcs;
      moves += MvEnpassant<Player> {lsb(pmoves), RelativeDir<E, Player>};
    }
    // Pinned knights cannot be moved. Only try to move unpinned knights.
    pcs = getBoard<Player, HRS>(p);
    while (pcs) {
      int pos = pop(pcs);
      pmoves  = KnightMoves[pos] & notself;
      while (pmoves) {
        moves += MvPiece<Player> {pos, pop(pmoves)};
      }
    }
    // Pinned diag sliders
    pcs = getBoard<Player, BSH, QEN>(p) & pins;
    while (pcs) {
      int pos = pop(pcs);
      pmoves  = bishopMoves(pos, nopins) & pins & notself;
      while (pmoves) {
        moves += MvPiece<Player> {pos, pop(pmoves)};
      }
    }
    // Pinned ortho sliders
    pcs = getBoard<Player, ROK, QEN>(p) & pins;
    while (pcs) {
      int pos = pop(pcs);
      pmoves  = rookMoves(pos, all) & notself;
      if (kingPos / 8 == pos / 8) {
        // Same rank as the king - restrict the movement to that rank.
        pmoves &= Rank[pos];
        while (pmoves) {
          moves += MvPiece<Player> {pos, pop(pmoves)};
        }
      }
      else if (kingPos % 8 == pos % 8) {
        // Same file as the king - restruct the movement to that file.
        pmoves &= File[pos];
        while (pmoves) {
          moves += MvPiece<Player> {pos, pop(pmoves)};
        }
      }
    }
    // Unpinned diag sliders
    pcs = getBoard<Player, BSH, QEN>(p) & nopins;
    while (pcs) {
      int pos = pop(pcs);
      pmoves  = bishopMoves(pos, all) & notself;
      while (pmoves) {
        moves += MvPiece<Player> {pos, pop(pmoves)};
      }
    }
    // Unpinned ortho sliders
    pcs = getBoard<Player, ROK, QEN>(p) & nopins;
    while (pcs) {
      int pos = pop(pcs);
      pmoves  = rookMoves(pos, all) & notself;
      while (pmoves) {
        moves += MvPiece<Player> {pos, pop(pmoves)};
      }
    }
    // Castling.
    static constexpr Castle CastleLong = Player == WHT ? Castle::W_LONG : Castle::B_LONG;
    static constexpr Castle CastleShort =
      Player == WHT ? Castle::W_SHORT : Castle::B_SHORT;
    static constexpr BitBoard CastleLongSafeMask =
      CastleSafeMask[std::countr_zero(uint8_t(CastleLong))];
    static constexpr BitBoard CastleLongEmptyMask =
      CastleEmptyMask[std::countr_zero(uint8_t(CastleLong))];
    static constexpr BitBoard CastleShortSafeMask =
      CastleSafeMask[std::countr_zero(uint8_t(CastleShort))];
    static constexpr BitBoard CastleShortEmptyMask =
      CastleEmptyMask[std::countr_zero(uint8_t(CastleShort))];
    Castle rights = p.castlingRights();
    if ((rights & CastleLong) && !(CastleLongEmptyMask & all) &&
        !(CastleLongSafeMask & unsafe)) {
      moves += MvCastleLong<Player> {};
    }
    if ((rights & CastleShort) && !(CastleShortEmptyMask & all) &&
        !(CastleShortSafeMask & unsafe)) {
      moves += MvCastleShort<Player> {};
    }
  }
}

}  // namespace potato

namespace std {

std::ostream& operator<<(std::ostream& os, const potato::Move& m);

}  // namespace std
