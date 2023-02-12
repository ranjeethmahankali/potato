#pragma once

#include <stdint.h>
#include <array>
#include <glm/glm.hpp>
#include <iterator>
#include <ostream>
#include <vector>

namespace potato {

using BitBoard = uint64_t;

enum Color : uint8_t
{
  BLK = 0,
  WHT = 8,  // Because third bit is used for color.
};

enum PieceType : uint8_t
{
  PWN = 1,
  HRS = 2,
  BSH = 3,
  ROK = 4,
  QEN = 5,
  KNG = 6,
};

enum Piece : uint8_t
{
  // Piece types. Calling a knight a horse to avoid confusion with king.
  B_PWN = 1,
  B_HRS = 2,
  B_BSH = 3,
  B_ROK = 4,
  B_QEN = 5,
  B_KNG = 6,
  W_PWN = 9,
  W_HRS = 10,
  W_BSH = 11,
  W_ROK = 12,
  W_QEN = 13,
  W_KNG = 14,
  // Empty
  NONE = 0,
};

enum Castle : uint8_t
{
  B_LONG  = 1,
  B_SHORT = 2,
  W_LONG  = 4,
  W_SHORT = 8,
};

static constexpr size_t NUniquePieces = 15;

Color     color(Piece pc);
PieceType type(Piece pc);
char      symbol(Piece pc);

class Position
{
public:
  Position();
  Position&       put(int pos, Piece pc);
  Position&       put(glm::ivec2 pos, Piece pc);
  Position&       remove(int pos);
  Position&       remove(glm::ivec2 pos);
  Position&       move(int from, int to);
  Position&       move(glm::ivec2 from, glm::ivec2 to);
  Piece           piece(int pos) const;
  Piece           piece(glm::ivec2 pos) const;
  void            clearEnpassant();
  void            clear();
  size_t          hash() const;
  std::string     fen() const;
  bool            valid() const;
  static Position fromFen(const std::string& fen);

private:
  void calcHash();

  std::array<Piece, 64>               mPieces;
  std::array<BitBoard, NUniquePieces> mBitBoards;
  size_t                              mHash            = 0;
  int                                 mHalfMoves       = 0;
  int                                 mMoveCounter     = 1;
  int                                 mEnPassantSquare = -1;
  Castle                              mCastlingRights  = Castle(0b1111);
  Color                               mTurn            = Color::WHT;
};

Position& currentPosition();
int       fileToX(char file);
int       rankToY(char rank);

}  // namespace potato

namespace std {

template<>
struct hash<potato::Position>
{
  size_t operator()(const potato::Position& b) const noexcept { return b.hash(); }
};

ostream& operator<<(ostream& os, const potato::Position& b);

}  // namespace std
