#include <Move.h>

namespace potato {

void Move::commit(Position& p)
{
  p.history().push({.mEnpassantSquare = p.enpassantSq()});
  p.setEnpassantSq(-1);
  p.history().push({.mCastlingRights = p.castlingRights()});
  std::visit([&p](auto& mv) { mv.commit(p); }, mVar);
}

void Move::revert(Position& p)
{
  std::visit([&p](auto& mv) { mv.revert(p); }, mVar);
  p.setCastlingRights(p.history().pop().mCastlingRights);
  p.setEnpassantSq(p.history().pop().mEnpassantSquare);
}

MoveList::MoveList()
    : mBuf()
    , mEnd(mBuf.data())
{}

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

void MoveList::operator+=(const Move& mv)
{
  *(mEnd++) = mv;
}

}  // namespace potato
