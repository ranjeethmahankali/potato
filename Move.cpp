#include <Move.h>
#include <bit>
#include <iostream>

namespace potato {

Move::Move(MoveType type, int from, int to)
    : mType(type)
    , mFrom(uint8_t(from))
    , mTo(uint8_t(to))
{}

void Move::assign(MoveType type, int from, int to)
{
  mType = type;
  mFrom = uint8_t(from);
  mTo   = uint8_t(to);
}

MoveType Move::type() const
{
  return mType;
}

int Move::from() const
{
  return int(mFrom);
}

int Move::to() const
{
  return int(mTo);
}

template<Color Player>
void commitMv(Position& p, MoveType mtype, int from, int to)
{
  static constexpr int       KngSideRookPos = Player == WHT ? 63 : Player == BLK ? 7 : -1;
  static constexpr int       QenSideRookPos = Player == WHT ? 56 : Player == BLK ? 0 : -1;
  static constexpr Castle    CastleShort    = Player == WHT   ? W_SHORT
                                              : Player == BLK ? B_SHORT
                                                              : Castle(0);
  static constexpr Castle    CastleLong     = Player == WHT   ? W_LONG
                                              : Player == BLK ? B_LONG
                                                              : Castle(0);
  static constexpr Direction Up             = RelativeDir<N, Player>;
  static constexpr int       HomeRank       = RelativeRank<Player, 0>;
  switch (mtype) {
  case SILENT:
    p.incrementHalfMoveCount();
    p.move(from, to);
    break;
  case CAPTURE:
    p.resetHalfMoveCount();
    p.history().push({.mPiece = p.piece(to)});
    p.move(from, to);
    break;
  case MV_KNG:
    p.revokeCastlingRights(CastleShort | CastleLong);
    p.history().push({.mPiece = p.piece(to)});
    p.move(from, to);
    break;
  case MV_ROK:
    if (from == KngSideRookPos) {
      p.revokeCastlingRights(CastleShort);
    }
    else if (from == QenSideRookPos) {
      p.revokeCastlingRights(CastleLong);
    }
    p.history().push({.mPiece = p.piece(to)});
    p.move(from, to);
    break;
  case PUSH:
    p.resetHalfMoveCount();
    p.move(from, to);
    break;
  case DBL_PUSH:
    p.resetHalfMoveCount();
    p.move(from, to).setEnpassantSq(to - Up);
    break;
  case ENPASSANT:
    p.resetHalfMoveCount();
    p.move(from, to).remove((from / 8) * 8 + to % 8);
    break;
  case PRM_HRS:
    p.resetHalfMoveCount();
    p.remove(from).put(to, Player | HRS);
    break;
  case PRM_BSH:
    p.resetHalfMoveCount();
    p.remove(from).put(to, Player | BSH);
    break;
  case PRM_ROK:
    p.resetHalfMoveCount();
    p.remove(from).put(to, Player | ROK);
    break;
  case PRM_QEN:
    p.resetHalfMoveCount();
    p.remove(from).put(to, Player | QEN);
    break;
  case PRC_HRS:
    p.resetHalfMoveCount();
    p.history().push({.mPiece = p.piece(to)});
    p.remove(from).put(to, Player | HRS);
    break;
  case PRC_BSH:
    p.resetHalfMoveCount();
    p.history().push({.mPiece = p.piece(to)});
    p.remove(from).put(to, Player | BSH);
    break;
  case PRC_ROK:
    p.resetHalfMoveCount();
    p.history().push({.mPiece = p.piece(to)});
    p.remove(from).put(to, Player | ROK);
    break;
  case PRC_QEN:
    p.resetHalfMoveCount();
    p.history().push({.mPiece = p.piece(to)});
    p.remove(from).put(to, Player | QEN);
    break;
  case CASTLE_SHORT:
    p.revokeCastlingRights(CastleShort | CastleLong);
    p.move(HomeRank * 8 + 4, HomeRank * 8 + 6).move(HomeRank * 8 + 7, HomeRank * 8 + 5);
    break;
  case CASTLE_LONG:
    p.revokeCastlingRights(CastleShort | CastleLong);
    p.move(HomeRank * 8 + 4, HomeRank * 8 + 2).move(HomeRank * 8 + 0, HomeRank * 8 + 3);
    break;
  default:  // Do Nothing.
    break;
  }
}

template<Color Player>
void revertMv(Position& p, MoveType mtype, int from, int to)
{
  static constexpr Color Enemy    = Player == WHT ? BLK : WHT;
  static constexpr int   HomeRank = RelativeRank<Player, 0>;
  switch (mtype) {
  case SILENT:
    p.move(to, from);
    break;
  case CAPTURE:
    p.move(to, from).put(to, p.history().pop().mPiece);
    break;
  case MV_KNG:
    p.move(to, from).put(to, p.history().pop().mPiece);
    break;
  case MV_ROK:
    p.move(to, from).put(to, p.history().pop().mPiece);
    break;
  case PUSH:
    p.move(to, from);
    break;
  case DBL_PUSH:
    p.move(to, from);
    break;
  case ENPASSANT:
    p.move(to, from).put((from / 8) * 8 + to % 8, Enemy | PWN);
    break;
  case PRM_HRS:  // Intentional fall through
  case PRM_BSH:  // Intentional fall through
  case PRM_ROK:  // Intentional fall through
  case PRM_QEN:
    p.remove(to).put(from, Player | PWN);
    break;
  case PRC_HRS:  // Intentional fall through
  case PRC_BSH:  // Intentional fall through
  case PRC_ROK:  // Intentional fall through
  case PRC_QEN:
    p.remove(to).put(from, Player | PWN).put(to, p.history().pop().mPiece);
    break;
  case CASTLE_SHORT:
    p.move(HomeRank * 8 + 6, HomeRank * 8 + 4).move(HomeRank * 8 + 5, HomeRank * 8 + 7);
    break;
  case CASTLE_LONG:
    p.move(HomeRank * 8 + 2, HomeRank * 8 + 4).move(HomeRank * 8 + 3, HomeRank * 8 + 0);
    break;
  default:
    break;
  }
}

void Move::commit(Position& p) const
{
  p.history().push({.mEnpassantSq = p.enpassantSq()});
  p.unsetEnpassantSq();
  p.history().push({.mCastlingRights = p.castlingRights()});
  p.history().push({.mCounter = p.moveCount()});
  p.history().push({.mCounter = p.halfMoveCount()});
  if (p.turn() == WHT) {
    commitMv<WHT>(p, mType, from(), to());
  }
  else if (p.turn() == BLK) {
    commitMv<BLK>(p, mType, from(), to());
  }
  p.switchTurn();
}

void Move::revert(Position& p) const
{
  p.switchTurn();
  if (p.turn() == WHT) {
    revertMv<WHT>(p, mType, from(), to());
  }
  else if (p.turn() == BLK) {
    revertMv<BLK>(p, mType, from(), to());
  }
  p.setHalfMoveCount(p.history().pop().mCounter);
  p.setMoveCount(p.history().pop().mCounter);
  p.setCastlingRights(p.history().pop().mCastlingRights);
  p.setEnpassantSq(p.history().pop().mEnpassantSq);
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

void MoveList::append(MoveType type, int from, int to)
{
  (mEnd++)->assign(type, from, to);
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
BitBoard pawnCapturesFromPos(int pos)
{
  if constexpr (Player == WHT) {
    return WhitePawnCaptures[pos];
  }
  else {
    return BlackPawnCaptures[pos];
  }
}

template<Color Player, Direction Dir>
void generatePawnCaptures(const Position& p,
                          MoveList&       moves,
                          BitBoard        pinned,
                          BitBoard        enemy,
                          BitBoard        mask,
                          int             kingPos)
{
  static constexpr BitBoard Rank7 = Rank[RelativeRank<Player, 6> * 8];
  // Ignore rank7 captures because they'll be handled as capture-promotions.
  auto pcs = getBoard<Player, PWN>(p) & ~Rank7;
  // not pined.
  auto pmoves = (shift<RelativeDir<Dir, Player>>(pcs & ~pinned)) & enemy;
  if (mask) {
    pmoves &= mask;
  }
  while (pmoves) {
    int pos = pop(pmoves);
    moves.append(CAPTURE, pos - RelativeDir<Dir, Player>, pos);
  }
  // pinned.
  pmoves = shift<RelativeDir<Dir, Player>>(pcs & pinned) & enemy;
  if (mask) {
    pmoves &= mask;
  }
  while (pmoves) {
    int pto   = pop(pmoves);
    int pfrom = pto - RelativeDir<Dir, Player>;
    if (LineMask[pfrom][kingPos] & OneHot[pto]) {
      moves.append(CAPTURE, pfrom, pto);
    }
  }
}

template<Color Player, Direction Dir>
void generatePawnCapturePromotions(const Position& p,
                                   MoveList&       moves,
                                   BitBoard        pinned,
                                   BitBoard        enemy,
                                   BitBoard        mask,
                                   int             kingPos)
{
  static constexpr BitBoard PromotionRank = Rank[RelativeRank<Player, 6> * 8];
  auto                      pawns  = getBoard<Player, PWN>(p) & PromotionRank & ~pinned;
  auto                      pmoves = shift<RelativeDir<Dir, Player>>(pawns) & enemy;
  auto pinnedPawns                 = getBoard<Player, PWN>(p) & PromotionRank & pinned;
  while (pinnedPawns) {
    int pos = pop(pinnedPawns);
    pmoves |=
      shift<RelativeDir<Dir, Player>>(OneHot[pos]) & enemy & LineMask[pos][kingPos];
  }
  if (mask) {
    pmoves &= mask;
  }
  while (pmoves) {
    int to   = pop(pmoves);
    int from = to - RelativeDir<Dir, Player>;
    moves.append(PRC_HRS, from, to);
    moves.append(PRC_BSH, from, to);
    moves.append(PRC_ROK, from, to);
    moves.append(PRC_QEN, from, to);
  }
}

template<Color Player, Direction Dir>
void generateEnpassant(const Position& p,
                       MoveList&       moves,
                       BitBoard        pinned,
                       int             kingPos,
                       BitBoard        all)
{
  static constexpr Color    Enemy         = Player == BLK ? WHT : BLK;
  static constexpr BitBoard EnpassantRank = Rank[RelativeRank<Player, 4> * 8];
  if (p.enpassantSq() == -1 ||
      p.piece(p.enpassantSq() + RelativeDir<S, Player>) != (Enemy | PWN)) {
    return;
  }
  auto pmoves = shift<RelativeDir<S, Player>>(
                  shift<RelativeDir<Direction(-Dir), Player>>(OneHot[p.enpassantSq()])) &
                getBoard<Player, PWN>(p);
  if (pmoves & pinned) {  // Expecting only one candidate.
    pmoves &= LineMask[p.enpassantSq()][kingPos];
  }
  if (pmoves) {
    int  from    = lsb(pmoves);
    int  to      = p.enpassantSq();
    int  target  = (from / 8) * 8 + to % 8;
    auto sliders = getBoard<Enemy, ROK, QEN>(p) & EnpassantRank;
    auto mask    = all & ~(OneHot[from] | OneHot[target]);
    bool safe    = true;
    while (sliders) {
      if (OneHot[kingPos] & rookMoves(pop(sliders), mask)) {
        safe = false;
        break;
      }
    }
    if (safe) {
      moves.append(ENPASSANT, from, to);
    }
  }
}

template<Color Player, int Steps>
void generatePawnPushMoves(const Position& p,
                           MoveList&       moves,
                           BitBoard        pinned,
                           BitBoard        empty,
                           BitBoard        mask,
                           int             kingPos)
{
  static_assert(Steps == 1 || Steps == 2, "Only single or double push allowed");
  static constexpr BitBoard  HomePawnRank = Rank[RelativeRank<Player, 1> * 8];
  static constexpr BitBoard  Rank7        = Rank[RelativeRank<Player, 6> * 8];
  static constexpr Direction Up           = RelativeDir<N, Player>;
  // Ignore rank 7 because their pushes will be handled as promotions.
  BitBoard pawns = getBoard<Player, PWN>(p) & ~Rank7;
  if constexpr (Steps == 2) {
    pawns &= HomePawnRank;
  }
  BitBoard pinnedPawns = pawns & pinned;
  pawns &= ~pinned;
  auto pmoves = shift<Up>(pawns) & empty;
  if constexpr (Steps == 2) {
    pmoves = shift<Up>(pmoves) & empty;
  }
  while (pinnedPawns) {
    int  pos         = pop(pinnedPawns);
    auto pinnedMoves = shift<Up>(OneHot[pos]) & empty;
    if constexpr (Steps == 2) {
      pinnedMoves = shift<Up>(pinnedMoves) & empty;
    }
    pinnedMoves &= LineMask[pos][kingPos];
    pmoves |= pinnedMoves;
  }
  if (mask) {
    pmoves &= mask;
  }
  while (pmoves) {
    int pos = pop(pmoves);
    if constexpr (Steps == 1) {
      moves.append(PUSH, pos - Steps * Up, pos);
    }
    else {
      moves.append(DBL_PUSH, pos - Steps * Up, pos);
    }
  }
}

template<Color Player>
void generatePawnPromotionMoves(const Position& p,
                                MoveList&       moves,
                                BitBoard        pinned,
                                BitBoard        empty,
                                BitBoard        mask)
{
  static constexpr Direction Up            = RelativeDir<N, Player>;
  static constexpr BitBoard  PromotionRank = Rank[RelativeRank<Player, 6> * 8];
  // Pinned pawns cannot promote.
  auto pcs    = getBoard<Player, PWN>(p) & PromotionRank & ~pinned;
  auto pmoves = shift<Up>(pcs) & empty;
  if (mask) {
    pmoves &= mask;
  }
  while (pmoves) {
    int to   = pop(pmoves);
    int from = to - Up;
    moves.append(PRM_HRS, from, to);
    moves.append(PRM_BSH, from, to);
    moves.append(PRM_ROK, from, to);
    moves.append(PRM_QEN, from, to);
  }
}

template<Color Player>
void generateDiagSlides(const Position& p,
                        MoveList&       moves,
                        BitBoard        pinned,
                        BitBoard        all,
                        BitBoard        notself,
                        BitBoard        mask,
                        int             kingPos)
{
  auto sliders = getBoard<Player, BSH, QEN>(p);
  while (sliders) {
    int  pos    = pop(sliders);
    auto pmoves = bishopMoves(pos, all) & notself;
    if (mask) {
      pmoves &= mask;
    }
    if (OneHot[pos] & pinned) {
      pmoves &= LineMask[kingPos][pos];
    }
    while (pmoves) {
      int dst = pop(pmoves);
      moves.append(p.piece(dst) ? CAPTURE : SILENT, pos, dst);
    }
  }
}

template<Color Player>
void generateOrthoSlides(const Position& p,
                         MoveList&       moves,
                         BitBoard        pinned,
                         BitBoard        all,
                         BitBoard        notself,
                         BitBoard        mask,
                         int             kingPos)
{
  auto sliders = getBoard<Player, ROK, QEN>(p);
  while (sliders) {
    int  pos    = pop(sliders);
    auto pmoves = rookMoves(pos, all) & notself;
    if (mask) {
      pmoves &= mask;
    }
    if (OneHot[pos] & pinned) {
      pmoves &= LineMask[kingPos][pos];
    }
    while (pmoves) {
      int dst = pop(pmoves);
      moves.append(p.piece(dst) ? CAPTURE : SILENT, pos, dst);
    }
  }
}

template<Color Player>
void generateMoves(const Position& p, MoveList& moves)
{
  static constexpr Direction Up           = RelativeDir<N, Player>;
  static constexpr BitBoard  HomePawnRank = Rank[RelativeRank<Player, 1> * 8];
  static constexpr Color     Enemy        = Player == BLK ? WHT : BLK;
  BitBoard                   self         = getAllBoards<Player>(p);
  BitBoard                   notself      = ~self;
  BitBoard                   enemy        = getAllBoards<Enemy>(p);
  BitBoard                   notenemy     = ~enemy;
  BitBoard                   all          = self | enemy;
  BitBoard                   empty        = ~all;
  BitBoard                   ourKing      = getBoard<Player, KNG>(p);
  BitBoard                   otherKing    = getBoard<Enemy, KNG>(p);
  int                        kingPos      = lsb(ourKing);
  int                        otherKingPos = lsb(otherKing);
  BitBoard                   unsafe       = 0;
  {  // Find all unsafe squares.
    BitBoard pcs = getBoard<Enemy, PWN>(p);
    unsafe = shift<RelativeDir<NE, Enemy>>(pcs) | shift<RelativeDir<NW, Enemy>>(pcs) |
             (KingMoves[otherKingPos] & notself);
    auto attackers = getBoard<Enemy, BSH, QEN>(p);
    auto allNoKing = all & (~ourKing);
    while (attackers) {
      unsafe |= bishopMoves(pop(attackers), allNoKing);
    }
    attackers = getBoard<Enemy, ROK, QEN>(p);
    while (attackers) {
      unsafe |= rookMoves(pop(attackers), allNoKing);
    }
    attackers = getBoard<Enemy, HRS>(p);
    while (attackers) {
      unsafe |= KnightMoves[pop(attackers)];
    }
    auto kmoves = KingMoves[kingPos] & (~(unsafe | self));
    while (kmoves) {
      int dst = pop(kmoves);
      moves.append(p.piece(dst) ? CAPTURE : SILENT, kingPos, dst);
    }
  }
  BitBoard pinned   = 0;
  BitBoard checkers = 0;
  {
    checkers |= (KnightMoves[kingPos] & getBoard<Enemy, HRS>(p)) |
                (pawnCapturesFromPos<Player>(kingPos) & getBoard<Enemy, PWN>(p));
    // Look for checks and pins from sliders.
    auto diags  = bishopMoves(kingPos, enemy) & getBoard<Enemy, BSH, QEN>(p);
    auto orthos = rookMoves(kingPos, enemy) & getBoard<Enemy, ROK, QEN>(p);
    // Slideres that are lined up with the king.
    auto sliders = diags | orthos;
    while (sliders) {
      int  spos = pop(sliders);
      auto line = Between[spos][kingPos];
      switch (std::popcount(line & self)) {
      case 0:  // The king is in check
        checkers |= OneHot[spos];
        break;
      case 1:
        pinned |= line & self;
        break;
      case 2:  // Not in check, not pinned.
        break;
      }
      if (OneHot[spos] & diags) {  // This is a diagonal slider.
        unsafe |= bishopMoves(spos, all);
      }
      else if (OneHot[spos] & orthos) {
        unsafe |= rookMoves(spos, all);
      }
    }
  }
  switch (std::popcount(checkers)) {
  case 0:  // Do nothing.
    break;
  case 1: {
    // Can block, or capture the checker. We already generated all possible king moves.
    // Try to capture.
    auto cpos     = lsb(checkers);
    bool isSlider = checkers & getBoard<Enemy, BSH, ROK, QEN>(p);
    auto line     = isSlider ? Between[cpos][kingPos] : 0;
    line |= checkers;
    // Pawn captures
    generatePawnCaptures<Player, NW>(p, moves, pinned, enemy, checkers, kingPos);
    generatePawnCaptures<Player, NE>(p, moves, pinned, enemy, checkers, kingPos);
    generatePawnCapturePromotions<Player, NW>(p, moves, pinned, enemy, checkers, kingPos);
    generatePawnCapturePromotions<Player, NE>(p, moves, pinned, enemy, checkers, kingPos);
    // Enpassant captures.
    if (p.enpassantSq() == cpos + RelativeDir<N, Player> &&
        p.piece(cpos) == (Enemy | PWN)) {
      auto attackers = shift<RelativeDir<E, Player>>(checkers) & getBoard<Player, PWN>(p);
      if (attackers & pinned) {
        attackers &= LineMask[p.enpassantSq()][kingPos];
      }
      while (attackers) {
        moves.append(ENPASSANT, pop(attackers), p.enpassantSq());
      }
      attackers = shift<RelativeDir<W, Player>>(checkers) & getBoard<Player, PWN>(p);
      if (attackers & pinned) {
        attackers &= LineMask[p.enpassantSq()][kingPos];
      }
      while (attackers) {
        moves.append(ENPASSANT, pop(attackers), p.enpassantSq());
      }
    }
    // Block with a pawn push
    generatePawnPushMoves<Player, 1>(p, moves, pinned, empty, line, kingPos);  // single
    generatePawnPushMoves<Player, 2>(p, moves, pinned, empty, line, kingPos);  // double
    // Block with a promotion
    generatePawnPromotionMoves<Player>(p, moves, pinned, empty, line);
    // Knight captures and blocks.
    auto attackers = getBoard<Player, HRS>(p) & ~pinned;
    while (attackers) {
      int  hpos   = pop(attackers);
      auto hmoves = KnightMoves[hpos] & (checkers | line) & notself;
      while (hmoves) {
        int dst = pop(hmoves);
        moves.append(p.piece(dst) ? CAPTURE : SILENT, hpos, dst);
      }
    }
    generateDiagSlides<Player>(p, moves, pinned, all, notself, line, kingPos);
    generateOrthoSlides<Player>(p, moves, pinned, all, notself, line, kingPos);
    // Generated all the moves to get out of check.
    // No more legal moves.
    return;
  }
  case 2:
    // The king must move to a safe square.
    // We already generated all possible king moves, so we stop looking for other mvoes.
    return;
  }
  {
    generatePawnPushMoves<Player, 1>(p, moves, pinned, empty, 0, kingPos);  // Single push
    generatePawnPushMoves<Player, 2>(p, moves, pinned, empty, 0, kingPos);  // Double push
    // Pawn captures
    generatePawnCaptures<Player, NE>(p, moves, pinned, enemy, 0, kingPos);
    generatePawnCaptures<Player, NW>(p, moves, pinned, enemy, 0, kingPos);
    // Pawn promotions.
    generatePawnPromotionMoves<Player>(p, moves, pinned, empty, 0);
    // Pawn capture promotions.
    generatePawnCapturePromotions<Player, NE>(p, moves, pinned, enemy, 0, kingPos);
    generatePawnCapturePromotions<Player, NW>(p, moves, pinned, enemy, 0, kingPos);
    // Enpassant
    generateEnpassant<Player, E>(p, moves, pinned, kingPos, all);
    generateEnpassant<Player, W>(p, moves, pinned, kingPos, all);
    // Pinned knights cannot be moved. Only try to move unpinned knights.
    auto pcs = getBoard<Player, HRS>(p) & ~pinned;
    while (pcs) {
      int  pos    = pop(pcs);
      auto pmoves = KnightMoves[pos] & notself;
      while (pmoves) {
        int dst = pop(pmoves);
        moves.append(p.piece(dst) ? CAPTURE : SILENT, pos, dst);
      }
    }
    // Sliders
    generateDiagSlides<Player>(p, moves, pinned, all, notself, 0, kingPos);
    generateOrthoSlides<Player>(p, moves, pinned, all, notself, 0, kingPos);
    // Castling.
    static constexpr Castle CastleLong = Player == WHT ? Castle::W_LONG : Castle::B_LONG;
    static constexpr Castle CastleShort =
      Player == WHT ? Castle::W_SHORT : Castle::B_SHORT;
    static constexpr BitBoard CastleLongSafeMask =
      CastleSafeMask[std::countr_zero(uint8_t(CastleLong))];
    static constexpr BitBoard CastleLongEmptyMask =
      CastleEmptyMask[std::countr_zero(uint8_t(CastleLong))];
    static constexpr BitBoard CastleShortSafeMask =
      CastleSafeMask[std::countr_zero(uint8_t(CastleShort))];
    static constexpr BitBoard CastleShortEmptyMask =
      CastleEmptyMask[std::countr_zero(uint8_t(CastleShort))];
    Castle rights = p.castlingRights();
    if ((rights & CastleLong) && !(CastleLongEmptyMask & all) &&
        !(CastleLongSafeMask & unsafe)) {
      moves.append(CASTLE_LONG, 0, 0);
    }
    if ((rights & CastleShort) && !(CastleShortEmptyMask & all) &&
        !(CastleShortSafeMask & unsafe)) {
      moves.append(CASTLE_SHORT, 0, 0);
    }
  }
}

void generateMoves(const Position& p, MoveList& moves)
{
  if (p.turn() == WHT) {
    generateMoves<WHT>(p, moves);
  }
  else if (p.turn() == BLK) {
    generateMoves<BLK>(p, moves);
  }
}

size_t perftInternal(Position& p, int depth)
{
  MoveList mlist;
  generateMoves(p, mlist);
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
  generateMoves(p, mlist);
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

std::ostream& operator<<(std::ostream& os, const Move& m)
{
  os << SquareCoord[m.from()] << SquareCoord[m.to()];
  switch (m.type()) {
  case PRM_HRS:
  case PRC_HRS:
    os << 'n';
    break;
  case PRM_BSH:
  case PRC_BSH:
    os << 'b';
    break;
  case PRM_ROK:
  case PRC_ROK:
    os << 'r';
    break;
  case PRM_QEN:
  case PRC_QEN:
    os << 'q';
    break;
  default:  // Do nothing.
    break;
  }
  return os;
}

}  // namespace std
