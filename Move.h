#pragma once

#include <Position.h>
#include <stdint.h>
#include <variant>

namespace potato {

enum Direction : int
{
  SW = -9,
  S  = -8,
  SE = -7,
  E  = 1,
  NE = 9,
  N  = 8,
  NW = 7,
  W  = -1
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
struct MvPush
{
  int mFrom;

  void commit(Position& p) const { p.move(mFrom, mFrom + RelativeDir<N, Player>); }
  void revert(Position& p) const { p.move(mFrom + RelativeDir<N, Player>, mFrom); }
};

template<Color Player>
struct MvDblPush
{
  int  mFrom;
  void commit(Position& p) const { p.move(mFrom, 2 * mFrom + RelativeDir<N, Player>); }
  void revert(Position& p) const { p.move(mFrom + 2 * RelativeDir<N, Player>, mFrom); }
};

template<Color Player>
struct MvPawnCapture
{
  int       mFrom;
  Direction mDir;
  Piece     mCaptured;

  void commit(Position& p)
  {
    int target = mFrom + relativeDir<Player>(mDir);
    mCaptured  = p.piece(target);
    p.move(mFrom, target);
  }

  void revert(Position& p)
  {
    int target = mFrom + relativeDir<Player>(mDir);
    p.move(target, mFrom).put(target, mCaptured);
  }
};

template<Color Player>
struct MvEnpassant
{
  int       mFrom;
  Direction mSide;

  void commit(Position& p)
  {
    int target = mFrom + relativeDir<Player>(mSide);
    int dest   = target + RelativeDir<N, Player>;
    p.move(mFrom, dest).remove(target);
  }
  void revert(Position& p)
  {
    static constexpr Color Enemy  = Color(~Player);
    int                    target = mFrom + relativeDir<Player>(mSide);
    int                    dest   = target + RelativeDir<N, Player>;
    p.move(dest, mFrom).put(target, Piece(uint8_t(Enemy) | PieceType::PWN));
  }
};

template<Color Player>
struct MvPawnPromote
{
  int   mFile;
  Piece mPromoted;

  void commit(Position& p)
  {
    p.remove(glm::ivec2 {mFile, RelativeRank<Player, 6>})
      .put(glm::ivec2 {mFile, RelativeRank<Player, 7>}, mPromoted);
  }
  void revert(Position& p) const
  {
    p.remove(glm::ivec2 {mFile, RelativeRank<Player, 7>})
      .put(glm::ivec2 {mFile, RelativeRank<Player, 6>},
           Piece(uint8_t(Player) | PieceType::PWN));
  }
};

template<Color Player>
struct MvPawnCapturePromote
{
  int       mFile;
  Direction mSide;
  Piece     mPromoted;
  Piece     mCaptured;

  void commit(Position& p)
  {
    glm::ivec2 dst = {mFile + relativeDir<Player>(mSide), RelativeRank<Player, 7>};
    mCaptured      = p.piece(dst);
    p.remove(glm::ivec2 {mFile, RelativeRank<Player, 6>}).put(dst, mPromoted);
  }
  void revert(Position& p) const
  {
    p.put(glm::ivec2 {mFile + relativeDir<Player>(mSide), RelativeRank<Player, 7>},
          mCaptured)
      .put(glm::ivec2 {mFile, RelativeRank<Player, 6>},
           Piece(uint8_t(Player) | PieceType::PWN));
  }
};

template<Color Player>
struct MvKnight
{
  int  mFrom;
  int  mTo;
  void commit(Position& p)
  {
    // TODO: Incomplete
  }
  void revert(Position& p) const
  {
    // TODO: Incomplete
  }
};

template<Color Player>
struct MvBishop
{
  int  mFrom;
  int  mTo;
  void commit(Position& p)
  {
    // TODO: Incomplete
  }
  void revert(Position& p) const
  {
    // TODO: Incomplete
  }
};

template<Color Player>
struct MvRook
{
  int  mFrom;
  int  mTo;
  void commit(Position& p)
  {
    // TODO: Incomplete
  }
  void revert(Position& p) const
  {
    // TODO: Incomplete
  }
};

template<Color Player>
struct MvQueen
{
  int  mFrom;
  int  mTo;
  void commit(Position& p)
  {
    // TODO: Incomplete
  }
  void revert(Position& p) const
  {
    // TODO: Incomplete
  }
};

template<Color Player>
struct MvKing
{
  Direction mDir;

  void commit(Position& p)
  {
    // TODO: Incomplete
  }
  void revert(Position& p) const
  {
    // TODO: Incomplete
  }
};

template<Color Player>
struct MvCastleShort
{
  void commit(Position& p)
  {
    // TODO: Incomplete
  }
  void revert(Position& p) const
  {
    // TODO: Incomplete
  }
};

template<Color Player>
struct MvCastleLong
{
  void commit(Position& p)
  {
    // TODO: Incomplete
  }
  void revert(Position& p) const
  {
    // TODO: Incomplete
  }
};

template<template<Color> typename... MoveTypes>
struct TMoveVariant
{
  using Type = std::variant<MoveTypes<Color::WHT>..., MoveTypes<Color::BLK>...>;
};

template<Color Player>
struct Move  // Wraps all moves in a variant.
{
private:
  using VariantType = typename TMoveVariant<MvPush,
                                            MvDblPush,
                                            MvPawnCapture,
                                            MvEnpassant,
                                            MvPawnPromote,
                                            MvPawnCapturePromote,
                                            MvKnight,
                                            MvBishop,
                                            MvRook,
                                            MvQueen,
                                            MvKing,
                                            MvCastleShort,
                                            MvCastleLong>::Type;
  VariantType mVar;
  uint8_t     mEnPassantSquare;
  Castle      mCastlingRights;

public:
  void commit(Position& p)
  {
    mEnPassantSquare = p.enpassantSq();
    p.setEnpassantSq(UINT8_MAX);
    mCastlingRights = p.castlingRights();
    std::visit([&p](const auto& mv) { mv.commit(p); }, mVar);
  }

  void revert(Position& p)
  {
    p.setEnpassantSq(mEnPassantSquare);
    p.setCastlingRights(mCastlingRights);
    std::visit([&p](const auto& mv) { mv.commit(p); }, mVar);
  }
};

}  // namespace potato
