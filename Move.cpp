#include <Move.h>
#include <bit>

namespace potato {

const Move::VariantType& Move::value() const
{
  return mVar;
}

void Move::commit(Position& p) const
{
  p.history().push({.mEnpassantSquare = p.enpassantSq()});
  p.setEnpassantSq(-1);
  p.history().push({.mCastlingRights = p.castlingRights()});
  std::visit([&p](auto& mv) { mv.commit(p); }, mVar);
}

void Move::revert(Position& p) const
{
  std::visit([&p](auto& mv) { mv.revert(p); }, mVar);
  p.setCastlingRights(p.history().pop().mCastlingRights);
  p.setEnpassantSq(p.history().pop().mEnpassantSquare);
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

}  // namespace potato

namespace std {

using namespace potato;
static constexpr auto coord = SquareCoord;

template<Color Player>
std::ostream& operator<<(std::ostream& os, const MvPiece<Player>& m)
{
  os << coord[m.mFrom] << coord[m.mTo];
  return os;
};

template<Color Player>
std::ostream& operator<<(std::ostream& os, const MvEnpassant<Player>& m)
{
  os << coord[m.mFrom] << 'x' << coord[m.dest()] << " e.p.";
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
  os << coord[m.mFile + RelativeRank<Player, 7> * 8] << '=' << symbol(m.mPromoted);
  return os;
};

template<Color Player>
std::ostream& operator<<(std::ostream& os, const MvCapturePromote<Player>& m)
{
  os << coord[int(m.mFile) + RelativeRank<Player, 6> * 8] << 'x'
     << coord[int(m.mFile) + RelativeRank<Player, 7> * 8 + relativeDir<Player>(m.mSide)]
     << '=' << symbol(m.mPromoted);
  return os;
};

template<Color Player>
std::ostream& operator<<(std::ostream& os, const MvCastleShort<Player>& m)
{
  os << "O-O";
  return os;
};

template<Color Player>
std::ostream& operator<<(std::ostream& os, const MvCastleLong<Player>& m)
{
  os << "O-O-O";
  return os;
};

std::ostream& operator<<(std::ostream& os, const Move& m)
{
  std::visit([&os](const auto& v) { os << v; }, m.value());
  return os;
}

}  // namespace std
