#pragma once

#include <Tables.h>
#include <Util.h>
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

static constexpr std::array<std::string_view, 64> SquareCoord = {{
  "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8", "a7", "b7", "c7", "d7", "e7",
  "f7", "g7", "h7", "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6", "a5", "b5",
  "c5", "d5", "e5", "f5", "g5", "h5", "a4", "b4", "c4", "d4", "e4", "f4", "g4",
  "h4", "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3", "a2", "b2", "c2", "d2",
  "e2", "f2", "g2", "h2", "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
}};

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

constexpr inline Piece operator|(Color c, PieceType type)
{
  return Piece(uint8_t(c) | type);
}

constexpr inline Piece operator|(PieceType type, Color c)
{
  return Piece(uint8_t(c) | type);
}

static constexpr size_t NUniquePieces = 15;

static constexpr std::array<int, NUniquePieces> MaterialValue = {
  {0, -1, -3, -3, -5, -9, -100, 0, 0, 1, 3, 3, 5, 9, 100}};

enum Castle : uint8_t
{
  B_LONG  = 1,
  B_SHORT = 2,
  W_LONG  = 4,
  W_SHORT = 8,
};

constexpr inline Castle operator|(Castle a, Castle b)
{
  return Castle(uint8_t(a) | b);
}

constexpr inline Castle operator&(Castle a, Castle b)
{
  return Castle(uint8_t(a) & b);
}

constexpr inline Color color(Piece pc)
{
  return Color(pc & 0b1000);
}

constexpr inline PieceType type(Piece pc)
{
  return PieceType(pc & 0b111);
}

char symbol(Piece pc);

class Position
{
public:
  struct State
  {
    uint16_t mMoveCount       = 1;
    uint8_t  mHalfMoveCount   = 0;
    int8_t   mEnPassantSquare = -1;
    Castle   mCastlingRights  = Castle(0b1111);

    bool operator==(const State&) const;
    bool operator!=(const State&) const;
  };

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
  void            unsetEnpassantSq();
  Castle          castlingRights() const;
  void            setCastlingRights(Castle c);
  void            revokeCastlingRights(Castle c);
  Color           turn() const;
  void            setTurn(Color turn);
  void            switchTurn();
  void            clear();
  size_t          hash() const;
  int             material() const;
  bool            valid() const;
  std::string     fen() const;
  void            incrementMoveCounter();
  void            incrementHalfMoveCount();
  void            resetHalfMoveCount();
  void            setMoveCount(int c);
  void            setHalfMoveCount(int c);
  int             moveCount() const;
  int             halfMoveCount() const;
  void            pushState();
  void            popState();
  void            freezeState();
  void            pushCapture(Piece p);
  Piece           popCapture();
  static Position empty();
  static Position fromFen(const std::string& fen);
  bool            operator==(const Position& other) const;
  bool            operator!=(const Position& other) const;

private:
  void calcHash();

  std::array<Piece, 64>               mPieces;
  std::array<BitBoard, NUniquePieces> mBitBoards;
  StaticVector<State, 32>             mState;
  StaticVector<Piece, 64>             mCaptured;
  size_t                              mHash     = 0;
  int                                 mMaterial = 0;
  Color                               mTurn     = Color::WHT;
};

void      writeBoard(BitBoard b, std::ostream& os);
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

ostream& operator<<(ostream& os, potato::Castle rights);

}  // namespace std
