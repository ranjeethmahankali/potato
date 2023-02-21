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
  if ((pc == W_ROK && mFrom == 56) || (old == W_ROK && mTo == 56)) {
    p.revokeCastlingRights(W_LONG);
  }
  else if ((pc == W_ROK && mFrom == 63) || (old == W_ROK && mTo == 63)) {
    p.revokeCastlingRights(W_SHORT);
  }
  else if ((pc == B_ROK && mFrom == 0) || (old == B_ROK && mTo == 0)) {
    p.revokeCastlingRights(B_LONG);
  }
  else if ((pc == B_ROK && mFrom == 7) || (old == B_ROK && mTo == 7)) {
    p.revokeCastlingRights(B_SHORT);
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
    moves += MvPiece {pos - RelativeDir<Dir, Player>, pos};
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
      moves += MvPiece {pfrom, pto};
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
    moves += MvCapturePromote<Player> {from, to, makePiece<Player>(QEN)};
    moves += MvCapturePromote<Player> {from, to, makePiece<Player>(ROK)};
    moves += MvCapturePromote<Player> {from, to, makePiece<Player>(BSH)};
    moves += MvCapturePromote<Player> {from, to, makePiece<Player>(HRS)};
  }
}

template<Color Player, Direction Dir>
void generateEnpassant(const Position& p, MoveList& moves, BitBoard pinned, int kingPos)
{
  static constexpr Color Enemy = Player == BLK ? WHT : BLK;
  if (p.enpassantSq() == -1 ||
      p.piece(p.enpassantSq() + RelativeDir<S, Player>) != makePiece<Enemy>(PWN)) {
    return;
  }
  auto pmoves = shift<RelativeDir<S, Player>>(
                  shift<RelativeDir<Direction(-Dir), Player>>(OneHot[p.enpassantSq()])) &
                getBoard<Player, PWN>(p);
  if (pmoves & pinned) {  // Expecting only one candidate.
    pmoves &= LineMask[p.enpassantSq()][kingPos];
  }
  if (pmoves) {
    moves += MvEnpassant<Player> {lsb(pmoves), Dir};
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
      moves += MvPiece {pos - Steps * Up, pos};
    }
    else {
      moves += MvDoublePush<Player> {pos - Steps * Up};
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
    uint8_t file = uint8_t(pop(pmoves) % 8);
    moves += MvPromote<Player> {file, makePiece<Player>(QEN)};
    moves += MvPromote<Player> {file, makePiece<Player>(ROK)};
    moves += MvPromote<Player> {file, makePiece<Player>(BSH)};
    moves += MvPromote<Player> {file, makePiece<Player>(HRS)};
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
      moves += MvPiece {pos, pop(pmoves)};
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
      moves += MvPiece {pos, pop(pmoves)};
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
      moves += MvPiece {kingPos, pop(kmoves)};
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
        p.piece(cpos) == makePiece<Enemy>(PWN)) {
      auto attackers = shift<RelativeDir<E, Player>>(checkers) & getBoard<Player, PWN>(p);
      if (attackers & pinned) {
        attackers &= LineMask[p.enpassantSq()][kingPos];
      }
      while (attackers) {
        moves += MvEnpassant<Player> {pop(attackers), W};
      }
      attackers = shift<RelativeDir<W, Player>>(checkers) & getBoard<Player, PWN>(p);
      if (attackers & pinned) {
        attackers &= LineMask[p.enpassantSq()][kingPos];
      }
      while (attackers) {
        moves += MvEnpassant<Player> {pop(attackers), E};
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
        moves += MvPiece {hpos, pop(hmoves)};
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
    generateEnpassant<Player, E>(p, moves, pinned, kingPos);
    generateEnpassant<Player, W>(p, moves, pinned, kingPos);
    // Pinned knights cannot be moved. Only try to move unpinned knights.
    auto pcs = getBoard<Player, HRS>(p) & ~pinned;
    while (pcs) {
      int  pos    = pop(pcs);
      auto pmoves = KnightMoves[pos] & notself;
      while (pmoves) {
        moves += MvPiece {pos, pop(pmoves)};
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
      moves += MvCastleLong<Player> {};
    }
    if ((rights & CastleShort) && !(CastleShortEmptyMask & all) &&
        !(CastleShortSafeMask & unsafe)) {
      moves += MvCastleShort<Player> {};
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
