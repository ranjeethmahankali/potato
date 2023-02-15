#pragma once

#include <Tables.h>
#include <stdint.h>
#include <array>
#include <cstdint>
#include <glm/glm.hpp>
#include <iterator>
#include <ostream>
#include <span>
#include <stack>
#include <vector>

namespace potato {

// clang-format off
enum Square : int
{
  A8 =  0, B8, C8, D8, E8, F8, G8, H8,
  A7 =  8, B7, C7, D7, E7, F7, G7, H7,
  A6 = 16, B6, C6, D6, E6, F6, G6, H6,
  A5 = 24, B5, C5, D5, E5, F5, G5, H5,
  A4 = 32, B4, C4, D4, E4, F4, G4, H4,
  A3 = 40, B3, C3, D3, E3, F3, G3, H3,
  A2 = 48, B2, C2, D2, E2, F2, G2, H2,
  A1 = 56, B1, C1, D1, E1, F1, G1, H1,
};
// clang-format on

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

static constexpr size_t NUniquePieces = 15;

enum Castle : uint8_t
{
  B_LONG  = 1,
  B_SHORT = 2,
  W_LONG  = 4,
  W_SHORT = 8,
};

union HistoryData
{
  Piece  mPiece;
  int    mEnpassantSquare;
  Castle mCastlingRights;
};

struct History : public std::stack<HistoryData>
{
  HistoryData pop();

private:
  using std::stack<HistoryData>::top;
};

Color     color(Piece pc);
PieceType type(Piece pc);
char      symbol(Piece pc);

class Position
{
public:
  Position();
  Position&       put(int pos, Piece pc);
  Position&       put(glm::ivec2 pos, Piece pc);
  Position&       put(std::span<const std::pair<int, Piece>> pieces);
  Position&       remove(int pos);
  Position&       remove(glm::ivec2 pos);
  Position&       move(int from, int to);
  Position&       move(glm::ivec2 from, glm::ivec2 to);
  Piece           piece(int pos) const;
  Piece           piece(glm::ivec2 pos) const;
  BitBoard        board(Piece p) const;
  int             enpassantSq() const;
  void            setEnpassantSq(int enp);
  Castle          castlingRights() const;
  void            setCastlingRights(Castle c);
  void            clear();
  size_t          hash() const;
  bool            valid() const;
  std::string     fen() const;
  History&        history();
  static Position fromFen(const std::string& fen);

private:
  void calcHash();

  std::array<Piece, 64>               mPieces;
  std::array<BitBoard, NUniquePieces> mBitBoards;
  History                             mHistory;
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
