#include <Move.h>
#include <bit>
#include <iostream>

namespace potato {

const Move::VariantType& Move::value() const
{
  return mVar;
}

void MvPiece::commit(Position& p) const
{
  Piece pc  = p.piece(mFrom);
  Piece old = p.piece(mTo);
  if (old != Piece::NONE || type(pc) == PWN) {
    // Captures and pawn pushes reset the halfmove counter.
    p.resetHalfMoveCount();
  }
  if (pc == W_ROK) {
    if (mFrom == 56) {
      p.revokeCastlingRights(W_LONG);
    }
    else if (mFrom == 63) {
      p.revokeCastlingRights(W_SHORT);
    }
  }
  else if (pc == B_ROK) {
    if (mFrom == 0) {
      p.revokeCastlingRights(B_LONG);
    }
    else if (mFrom == 7) {
      p.revokeCastlingRights(B_SHORT);
    }
  }
  else if (pc == W_KNG) {
    p.revokeCastlingRights(Castle(W_LONG | W_SHORT));
  }
  else if (pc == B_KNG) {
    p.revokeCastlingRights(Castle(B_LONG | B_SHORT));
  }
  p.history().push({.mPiece = old});
  p.move(mFrom, mTo);
}

void MvPiece::revert(Position& p) const
{
  p.move(mTo, mFrom).put(mTo, p.history().pop().mPiece);
}

void Move::commit(Position& p) const
{
  p.history().push({.mEnpassantSquare = p.enpassantSq()});
  p.setEnpassantSq(-1);
  p.history().push({.mCastlingRights = p.castlingRights()});
  p.history().push({.mCounter = p.moveCount()});
  p.history().push({.mCounter = p.halfMoveCount()});
  p.incrementHalfMoveCount();
  std::visit([&p](auto& mv) { mv.commit(p); }, mVar);
  if (p.turn() == BLK) {
    p.incrementMoveCounter();
  }
  p.switchTurn();
}

void Move::revert(Position& p) const
{
  std::visit([&p](auto& mv) { mv.revert(p); }, mVar);
  p.setHalfMoveCount(p.history().pop().mCounter);
  p.setMoveCount(p.history().pop().mCounter);
  p.setCastlingRights(p.history().pop().mCastlingRights);
  p.setEnpassantSq(p.history().pop().mEnpassantSquare);
  p.switchTurn();
}

MoveList::MoveList()
    : mBuf()
    , mEnd(mBuf.data())
{}

MoveList::MoveList(const MoveList& other)
    : mBuf(other.mBuf)
    , mEnd(mBuf.data() + other.size())
{}

MoveList::MoveList(MoveList&& other)
    : mBuf(other.mBuf)
    , mEnd(mBuf.data() + other.size())
{
  other.clear();
}

const MoveList& MoveList::operator=(const MoveList& other)
{
  mBuf = other.mBuf;
  mEnd = mBuf.data() + other.size();
  return *this;
}
const MoveList& MoveList::operator=(MoveList&& other)
{
  mBuf = other.mBuf;
  mEnd = mBuf.data() + other.size();
  other.clear();
  return *this;
}

const Move* MoveList::begin() const
{
  return mBuf.data();
}

const Move* MoveList::end() const
{
  return mEnd;
}

size_t MoveList::size() const
{
  return size_t(mEnd - mBuf.data());
}

void MoveList::clear()
{
  mEnd = mBuf.data();
}

const Move& MoveList::operator[](size_t i) const
{
  return mBuf[i];
}

int pop(BitBoard& b)
{
  int shift = std::countr_zero(b);
  b &= ~OneHot[shift];
  return shift;
}

int lsb(BitBoard b)
{
  return std::countr_zero(b);
}

BitBoard reversed(BitBoard b)
{
  b = (b & 0x5555555555555555) << 1 | (b >> 1) & 0x5555555555555555;
  b = (b & 0x3333333333333333) << 2 | (b >> 2) & 0x3333333333333333;
  b = (b & 0x0f0f0f0f0f0f0f0f) << 4 | (b >> 4) & 0x0f0f0f0f0f0f0f0f;
  b = (b & 0x00ff00ff00ff00ff) << 8 | (b >> 8) & 0x00ff00ff00ff00ff;
  return (b << 48) | ((b & 0xffff0000) << 16) | ((b >> 16) & 0xffff0000) | (b >> 48);
}

BitBoard sliderMoves(int sq, BitBoard blockers, BitBoard mask)
{
  // Use the hyperbola quintissence algorithm.
  BitBoard pc     = OneHot[sq];
  BitBoard masked = blockers & mask;
  return ((masked - 2 * pc) ^ reversed(reversed(masked) - 2 * OneHot[63 - sq])) & mask;
}

BitBoard bishopMoves(int sq, BitBoard blockers)
{
  return sliderMoves(sq, blockers, Diagonal[sq]) |
         sliderMoves(sq, blockers, AntiDiagonal[sq]);
}

BitBoard rookMoves(int sq, BitBoard blockers)
{
  return sliderMoves(sq, blockers, File[sq]) | sliderMoves(sq, blockers, Rank[sq]);
}

BitBoard queenMoves(int sq, BitBoard blockers)
{
  return bishopMoves(sq, blockers) | rookMoves(sq, blockers);
}

size_t perftInternal(Position& p, int depth)
{
  MoveList mlist;
  if (p.turn() == WHT) {
    generateMoves<WHT>(p, mlist);
  }
  else {
    generateMoves<BLK>(p, mlist);
  }
  if (depth == 1) {
    return mlist.size();
  }
  size_t total = 0;
  for (const auto& m : mlist) {
    m.commit(p);
    total += perftInternal(p, depth - 1);
    m.revert(p);
  }
  return total;
}

void perft(const Position& pOriginal, int depth)
{
  Position p = pOriginal;
  MoveList mlist;
  if (p.turn() == WHT) {
    generateMoves<WHT>(p, mlist);
  }
  else {
    generateMoves<BLK>(p, mlist);
  }
  size_t total = 0;
  for (const auto& m : mlist) {
    m.commit(p);
    size_t n = depth == 1 ? 1 : perftInternal(p, depth - 1);
    std::cout << m << ": " << n << std::endl;
    total += n;
    m.revert(p);
  }
  std::cout << std::endl << "Total: " << total << std::endl;
}

}  // namespace potato

namespace std {

using namespace potato;
static constexpr auto coord = SquareCoord;

std::ostream& operator<<(std::ostream& os, const MvPiece& m)
{
  os << coord[m.mFrom] << coord[m.mTo];
  return os;
};

template<Color Player>
std::ostream& operator<<(std::ostream& os, const MvEnpassant<Player>& m)
{
  os << coord[m.mFrom] << coord[m.dest()];
  return os;
};

template<Color Player>
std::ostream& operator<<(std::ostream& os, const MvDoublePush<Player>& m)
{
  os << coord[m.mFrom] << coord[m.dest()];
  return os;
};

template<Color Player>
std::ostream& operator<<(std::ostream& os, const MvPromote<Player>& m)
{
  os << coord[m.mFile + RelativeRank<Player, 6> * 8]
     << coord[m.mFile + RelativeRank<Player, 7> * 8] << symbol(m.mPromoted);
  return os;
};

template<Color Player>
std::ostream& operator<<(std::ostream& os, const MvCapturePromote<Player>& m)
{
  os << coord[m.mFrom] << coord[m.mTo] << symbol(m.mPromoted);
  return os;
};

template<Color Player>
std::ostream& operator<<(std::ostream& os, const MvCastleShort<Player>& m)
{
  if constexpr (Player == WHT) {
    os << "e1g1";
  }
  else if constexpr (Player == BLK) {
    os << "e8g8";
  }
  else {
    os << "O-O";
  }
  return os;
};

template<Color Player>
std::ostream& operator<<(std::ostream& os, const MvCastleLong<Player>& m)
{
  if constexpr (Player == WHT) {
    os << "e1c1";
  }
  else if constexpr (Player == BLK) {
    os << "e8c8";
  }
  else {
    os << "O-O";
  }
  return os;
};

std::ostream& operator<<(std::ostream& os, const Move& m)
{
  std::visit([&os](const auto& v) { os << v; }, m.value());
  return os;
}

}  // namespace std
