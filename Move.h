#pragma once

#include <Position.h>
#include <stdint.h>
#include <variant>
#include "Tables.h"

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
                                            MvEnpassant,
                                            MvPromote,
                                            MvCapturePromote,
                                            MvCastleShort,
                                            MvCastleLong>::Type;
  VariantType mVar;

public:
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
  void        operator+=(const Move& mv);

private:
  static constexpr size_t MaxMoves = 256;
  std::array<Move, 256>   mBuf;
  Move*                   mEnd;
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

template<Color Player>
BitBoard getAllPieces(const Position& p)
{
  static constexpr uint8_t color = uint8_t(Player);
  return p.board(Piece(color | PieceType::PWN)) | p.board(Piece(color | PieceType::HRS)) |
         p.board(Piece(color | PieceType::BSH)) | p.board(Piece(color | PieceType::ROK)) |
         p.board(Piece(color | PieceType::QEN)) | p.board(Piece(color | PieceType::KNG));
}

template<Color Player>
BitBoard getDiagSliders(const Position& p)
{
  static constexpr uint8_t color = uint8_t(Player);
  return p.board(Piece(color | PieceType::BSH)) | p.board(Piece(color | PieceType::QEN));
}

template<Color Player>
BitBoard getOrthoSliders(const Position& p)
{
  static constexpr uint8_t color = uint8_t(Player);
  return p.board(Piece(color | PieceType::ROK)) | p.board(Piece(color | PieceType::QEN));
}

template<Color Player>
void generateMoves(const Position& p, MoveList& moves)
{
  static constexpr Color Enemy = Player == BLK ? WHT : BLK;
  // TODO: Incomplete.
}

}  // namespace potato
