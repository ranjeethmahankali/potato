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

template<Color Player>
Piece makePiece(PieceType type)
{
  return Piece(uint8_t(Player) | type);
}

struct MvPiece
{
  int mFrom;
  int mTo;

  void commit(Position& p) const;
  void revert(Position& p) const;
};

template<Color Player>
struct MvEnpassant
{
  int       mFrom;
  Direction mSide;

  int  target() const { return mFrom + relativeDir<Player>(mSide); }
  int  dest() const { return target() + RelativeDir<N, Player>; }
  void commit(Position& p) const
  {
    p.resetHalfMoveCount();
    p.move(mFrom, dest()).remove(target());
  }
  void revert(Position& p) const
  {
    static constexpr Color Enemy = Player == WHT ? BLK : WHT;
    p.move(dest(), mFrom).put(target(), makePiece<Enemy>(PWN));
  }
};

template<Color Player>
struct MvDoublePush
{
  int mFrom;

  int dest() const { return mFrom + 2 * RelativeDir<N, Player>; }

  void commit(Position& p) const
  {
    p.resetHalfMoveCount();
    int d = dest();
    p.move(mFrom, d);
    p.setEnpassantSq(d - RelativeDir<N, Player>);
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
    p.resetHalfMoveCount();
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
struct MvCapturePromote : MvPiece
{
  Piece mPromoted;

  void commit(Position& p) const
  {
    p.resetHalfMoveCount();
    MvPiece::commit(p);
    p.put(mTo, mPromoted);
  }
  void revert(Position& p) const
  {
    MvPiece::revert(p);
    p.put(mFrom, makePiece<Player>(PWN));
  }
};

template<Color Player>
struct MvCastleShort
{
  static constexpr int    Rank   = RelativeRank<Player, 0>;
  static constexpr Castle Rights = Castle(Player == BLK   ? 0b11
                                          : Player == WHT ? 0b1100
                                                          : 0);

  void commit(Position& p) const
  {
    p.revokeCastlingRights(Rights);
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
  static constexpr int    Rank   = RelativeRank<Player, 0>;
  static constexpr Castle Rights = Castle(Player == BLK   ? 0b11
                                          : Player == WHT ? 0b1100
                                                          : 0);

  void commit(Position& p) const
  {
    p.revokeCastlingRights(Rights);
    p.move({4, Rank}, {2, Rank}).move({0, Rank}, {3, Rank});
  }
  void revert(Position& p) const
  {
    p.move({2, Rank}, {4, Rank}).move({3, Rank}, {0, Rank});
  }
};

struct Move  // Wraps all moves in a variant.
{
private:
  using VariantType = std::variant<MvPiece,
                                   MvDoublePush<BLK>,
                                   MvDoublePush<WHT>,
                                   MvEnpassant<BLK>,
                                   MvEnpassant<WHT>,
                                   MvPromote<BLK>,
                                   MvPromote<WHT>,
                                   MvCapturePromote<BLK>,
                                   MvCapturePromote<WHT>,
                                   MvCastleShort<BLK>,
                                   MvCastleShort<WHT>,
                                   MvCastleLong<BLK>,
                                   MvCastleLong<WHT>>;
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
  const Move&     operator[](size_t i) const;

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
  return (p.board(makePiece<Player>(Types)) | ...);
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
