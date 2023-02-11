#pragma once

#include <stdint.h>
#include <array>
#include <glm/glm.hpp>
#include <iterator>
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

class Position
{
public:
  template<bool IsConst>
  class IteratorT
  {
    using BoardRef = std::conditional_t<IsConst, const Position&, Position&>;

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

    PtrType operator->() { return mBoard.ptr(mPos); }

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

    glm::ivec2 pos() const { return mPos; }

  private:
    glm::ivec2 mPos;
    BoardRef   mBoard;
  };

  using Iterator      = IteratorT<false>;
  using ConstIterator = IteratorT<true>;

  Position();
  Position(const Position& other);
  bool            operator==(const Position& other) const;
  bool            operator!=(const Position& other) const;
  uint8_t&        piece(glm::ivec2 pos);
  uint8_t         piece(glm::ivec2 pos) const;
  uint8_t*        ptr(glm::ivec2 pos);
  const uint8_t*  ptr(glm::ivec2 pos) const;
  glm::ivec2      first() const;
  glm::ivec2      next(glm::ivec2 pos) const;
  glm::ivec2      last() const;
  Position&       move(glm::ivec2 from, glm::ivec2 to);
  Position&       setMask(glm::ivec2 pos, uint8_t mask);
  Position&       clearMask(glm::ivec2 pos, uint8_t mask);
  Position&       setPiece(glm::ivec2 pos, uint8_t pc);
  Position&       clearEnpassant();
  void            genMoves(std::vector<Position>& dst, uint8_t turn) const;
  Iterator        begin();
  ConstIterator   begin() const;
  Iterator        end();
  ConstIterator   end() const;
  void            clear();
  size_t          zobristHash() const;
  bool            inCheck(uint8_t color) const;
  std::string     fen() const;
  static Position fromFen(const std::string& fen);

private:
  union
  {
    std::array<uint8_t, 64> mPieces;
    std::array<uint64_t, 8> mRows;
  };
  int     mHalfMoves = 1;
  int     mFullMoves = 0;
  uint8_t mTurn      = Piece::WHT;
};

Position& currentPosition();
int       fileToX(char file);
int       rankToY(char rank);

}  // namespace potato

namespace std {

template<>
struct hash<potato::Position>
{
  size_t operator()(const potato::Position& b) const noexcept { return b.zobristHash(); }
};

ostream& operator<<(ostream& os, const potato::Position& b);

template<bool IsConst>
struct iterator_traits<potato::Position::IteratorT<IsConst>>
{
  using iterator_type     = uint8_t;
  using iterator_category = std::forward_iterator_tag;
  using value_type        = uint8_t;
  using pointer           = typename potato::Position::IteratorT<IsConst>::PtrType;
  using reference         = std::conditional_t<IsConst, const uint8_t&, uint8_t&>;
};

}  // namespace std
