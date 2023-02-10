#pragma once

#include <stdint.h>
#include <array>
#include <glm/glm.hpp>
#include <ostream>
#include <vector>

namespace potato {

struct Piece
{
  // Colors
  static constexpr uint8_t BLK = 0b01'000;
  static constexpr uint8_t WHT = 0b10'000;
  // Piece types. Calling a knight a horse to avoid confusion with king.
  static constexpr uint8_t PWN = 1;
  static constexpr uint8_t BSH = 2;
  static constexpr uint8_t HRS = 3;
  static constexpr uint8_t ROK = 4;
  static constexpr uint8_t QEN = 5;
  static constexpr uint8_t KNG = 6;
  // Empty
  static constexpr uint8_t NONE = 0;
  // States
  static constexpr uint8_t CASTLE    = 0b01'00'000;
  static constexpr uint8_t ENPASSANT = 0b10'00'000;

  static uint8_t color(uint8_t piece);
  static uint8_t type(uint8_t piece);
};

class Board
{
public:
  template<bool IsConst>
  class IteratorT
  {
    using BoardRef = std::conditional_t<IsConst, const Board&, Board&>;

  public:
    using PtrType   = std::conditional_t<IsConst, const uint8_t*, uint8_t*>;
    using DerefType = std::conditional_t<IsConst, uint8_t, uint8_t&>;

    IteratorT(BoardRef board, glm::ivec2 pos)
        : mBoard(board)
        , mPos(pos)
    {}

    explicit IteratorT(BoardRef board)
        : IteratorT(board, board.first())
    {}

    DerefType operator*() { return mBoard.piece(mPos); }

    PtrType operator->() { return mBoard.mPieces(mPos); }

    bool operator==(const IteratorT<IsConst>& other) const
    {
      return &(other.mBoard) == &mBoard && other.mPos == mPos;
    }

    bool operator!=(const IteratorT<IsConst>& other) const { return !(*this == other); }

    const IteratorT<IsConst>& operator++()
    {
      mPos = mBoard.next(mPos);
      return *this;
    }

    IteratorT<IsConst> operator++(int)
    {
      IteratorT<IsConst> copy = *this;
      ++(*this);
      return copy;
    }

  private:
    glm::ivec2 mPos;
    BoardRef   mBoard;
  };

  using Iterator      = IteratorT<false>;
  using ConstIterator = IteratorT<true>;

  Board();
  Board(const Board& other);
  uint8_t&       piece(glm::ivec2 pos);
  const uint8_t& piece(glm::ivec2 pos) const;
  glm::ivec2     first() const;
  glm::ivec2     next(glm::ivec2 pos) const;
  glm::ivec2     last() const;
  Board&         move(glm::ivec2 from, glm::ivec2 to);
  Board&         setMask(glm::ivec2 pos, uint8_t mask);
  Board&         clearMask(glm::ivec2 pos, uint8_t mask);
  Board&         setPiece(glm::ivec2 pos, uint8_t pc);
  Board&         clearEnpassant();
  void           genMoves(std::vector<Board>& dst, uint8_t turn) const;
  Iterator       begin();
  ConstIterator  begin() const;
  Iterator       end();
  ConstIterator  end() const;

private:
  union
  {
    std::array<uint8_t, 64> mPieces;
    std::array<uint64_t, 8> mRows;
  };
};

}  // namespace potato

namespace std {
ostream& operator<<(ostream& os, const potato::Board& b);
}
