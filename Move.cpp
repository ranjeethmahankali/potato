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
  static constexpr Color     Enemy          = Player == WHT ? BLK : WHT;
  static constexpr int       KngSideRookPos = Player == WHT ? 63 : Player == BLK ? 7 : -1;
  static constexpr int       QenSideRookPos = Player == WHT ? 56 : Player == BLK ? 0 : -1;
  static constexpr Castle    CastleShort    = Player == WHT   ? W_SHORT
                                              : Player == BLK ? B_SHORT
                                                              : Castle(0);
  static constexpr Castle    CastleLong     = Player == WHT   ? W_LONG
                                              : Player == BLK ? B_LONG
                                                              : Castle(0);
  static constexpr Castle    EnemyCastleShort = Player == WHT   ? B_SHORT
                                                : Player == BLK ? W_SHORT
                                                                : Castle(0);
  static constexpr Castle    EnemyCastleLong  = Player == WHT   ? B_LONG
                                                : Player == BLK ? W_LONG
                                                                : Castle(0);
  static constexpr Direction Up               = RelativeDir<N, Player>;
  static constexpr int       HomeRank         = RelativeRank<Player, 0>;
  static constexpr int       EnemyHomeRank    = RelativeRank<Player, 7>;
  bool                       isCapture        = mtype & CAPTURE;
  mtype                                       = MoveType(mtype & ~CAPTURE);
  if (isCapture) {
    if (p.piece(to) == (Enemy | ROK)) {
      if (to == EnemyHomeRank * 8) {
        p.revokeCastlingRights(EnemyCastleLong);
      }
      else if (to == EnemyHomeRank * 8 + 7) {
        p.revokeCastlingRights(EnemyCastleShort);
      }
    }
    p.pushCapture(p.piece(to));
  }
  switch (mtype) {
  case MV_KNG:
    p.incrementHalfMoveCount();
    p.revokeCastlingRights(CastleShort | CastleLong);
    p.move(from, to);
    break;
  case MV_ROK:
    p.incrementHalfMoveCount();
    if (from == KngSideRookPos) {
      p.revokeCastlingRights(CastleShort);
    }
    else if (from == QenSideRookPos) {
      p.revokeCastlingRights(CastleLong);
    }
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
    p.remove(from).put(to, Player | HRS);
    break;
  case PRC_BSH:
    p.resetHalfMoveCount();
    p.remove(from).put(to, Player | BSH);
    break;
  case PRC_ROK:
    p.resetHalfMoveCount();
    p.remove(from).put(to, Player | ROK);
    break;
  case PRC_QEN:
    p.resetHalfMoveCount();
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
  case OTHER:
  default:  // Intentional fallthrough
    p.incrementHalfMoveCount();
    p.move(from, to);
    break;
  }
  if (isCapture) {
    p.resetHalfMoveCount();
  }
}

template<Color Player>
void revertMv(Position& p, MoveType mtype, int from, int to)
{
  static constexpr Color Enemy     = Player == WHT ? BLK : WHT;
  static constexpr int   HomeRank  = RelativeRank<Player, 0>;
  bool                   isCapture = mtype & CAPTURE;
  mtype                            = MoveType(mtype & ~CAPTURE);
  switch (mtype) {
  case MV_KNG:
  case MV_ROK:
  case PUSH:
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
    p.remove(to).put(from, Player | PWN);
    break;
  case CASTLE_SHORT:
    p.move(HomeRank * 8 + 6, HomeRank * 8 + 4).move(HomeRank * 8 + 5, HomeRank * 8 + 7);
    break;
  case CASTLE_LONG:
    p.move(HomeRank * 8 + 2, HomeRank * 8 + 4).move(HomeRank * 8 + 3, HomeRank * 8 + 0);
    break;
  case OTHER:  // Intentional fall through
  default:
    p.move(to, from);
    break;
  }
  if (isCapture) {
    p.put(to, p.popCapture());
  }
}

void Move::commit(Position& p) const
{
  p.pushState();
  p.unsetEnpassantSq();
  if (p.turn() == WHT) {
    commitMv<WHT>(p, mType, from(), to());
  }
  else if (p.turn() == BLK) {
    commitMv<BLK>(p, mType, from(), to());
    p.incrementMoveCounter();
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
  p.popState();
}

std::string Move::algebraic() const
{
  std::string out = std::string(SquareCoord[from()]);
  out += SquareCoord[to()];
  switch (type() & ~CAPTURE) {
  case PRM_HRS:
  case PRC_HRS:
    out.push_back('n');
    break;
  case PRM_BSH:
  case PRC_BSH:
    out.push_back('b');
    break;
  case PRM_ROK:
  case PRC_ROK:
    out.push_back('r');
    break;
  case PRM_QEN:
  case PRC_QEN:
    out.push_back('q');
    break;
  default:  // Do nothing.
    break;
  }
  return out;
}

bool Move::operator==(const Move& other)
{
  return mType == other.mType && mFrom == other.mFrom && mTo == other.mTo;
}

bool Move::operator!=(const Move& other)
{
  return !(*this == other);
}

void MoveList::append(MoveType type, int from, int to, bool isCapture)
{
  if (isCapture) {
    type = type | CAPTURE;
  }
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

static constexpr std::array<uint64_t, (1 << 16)> reversed16()
{
  std::array<uint64_t, (1 << 16)> out;
  for (size_t i = 0; i < out.size(); ++i) {
    uint16_t v = uint16_t(i);
    v      = ((v >> 1) & 0x55555555) | ((v & 0x55555555) << 1);  // swap even and odd bits
    v      = ((v >> 2) & 0x33333333) | ((v & 0x33333333) << 2);  // swap consecutive pairs
    v      = ((v >> 4) & 0x0F0F0F0F) | ((v & 0x0F0F0F0F) << 4);  // swap nibbles
    v      = ((v >> 8) & 0x00FF00FF) | ((v & 0x00FF00FF) << 8);  // swap bytes
    out[i] = uint64_t(v);
  }
  return out;
}

BitBoard reversed(BitBoard b)
{
  static constexpr std::array<uint64_t, (1 << 16)> sReversed16 = reversed16();
  return (sReversed16[0xffff & b] << 48) | (sReversed16[0xffff & (b >> 16)] << 32) |
         (sReversed16[0xffff & (b >> 32)] << 16) | (sReversed16[0xffff & (b >> 48)]);
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
    moves.append(CAPTURE | OTHER, pos - RelativeDir<Dir, Player>, pos);
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
      moves.append(CAPTURE | OTHER, pfrom, pto);
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
    moves.append(CAPTURE | PRC_HRS, from, to);
    moves.append(CAPTURE | PRC_BSH, from, to);
    moves.append(CAPTURE | PRC_ROK, from, to);
    moves.append(CAPTURE | PRC_QEN, from, to);
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
    auto sliders = getBoard<Enemy, ROK, QEN>(p) & EnpassantRank & Rank[kingPos];
    bool safe    = true;
    while (sliders) {
      int spos = pop(sliders);
      if ((Between[kingPos][spos] & all) == (OneHot[target] | OneHot[from])) {
        // This means the attacking pawn and the attacked pawn are the only pieces
        // blocking a check. So enpassant is illegal.
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
      moves.append(OTHER, pos, dst, p.piece(dst));
    }
  }
}

template<Color Player, PieceType PType>
void generateOrthoSlidesHelper(const Position& p,
                               MoveList&       moves,
                               BitBoard        pinned,
                               BitBoard        all,
                               BitBoard        notself,
                               BitBoard        mask,
                               int             kingPos,
                               MoveType        mtype)
{
  auto sliders = getBoard<Player, PType>(p);
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
      moves.append(mtype, pos, dst, p.piece(dst));
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
  generateOrthoSlidesHelper<Player, ROK>(
    p, moves, pinned, all, notself, mask, kingPos, MV_ROK);
  generateOrthoSlidesHelper<Player, QEN>(
    p, moves, pinned, all, notself, mask, kingPos, OTHER);
}

/**
 * @brief Generate legal moves for a position.
 *
 * @tparam Player The color to move.
 * @param p The position.
 * @param moves Legal moves will be written to this list. Previous contents will be
 * erased.
 * @return bool Flag indicating if the player's king is in check. This may be used to
 * identify checkmate / stalemate.
 */
template<Color Player>
[[nodiscard]] bool generateMoves(const Position& p, MoveList& moves)
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
  moves.clear();
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
      moves.append(MV_KNG, kingPos, dst, p.piece(dst));
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
        moves.append(OTHER, hpos, dst, p.piece(dst));
      }
    }
    generateDiagSlides<Player>(p, moves, pinned, all, notself, line, kingPos);
    generateOrthoSlides<Player>(p, moves, pinned, all, notself, line, kingPos);
    // Generated all the moves to get out of check.
    // No more legal moves.
    return true;
  }
  case 2:
    // The king must move to a safe square.
    // We already generated all possible king moves, so we stop looking for other mvoes.
    return true;
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
        moves.append(OTHER, pos, dst, p.piece(dst));
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
      moves.append(
        CASTLE_LONG, RelativeRank<Player, 0> * 8 + 4, RelativeRank<Player, 0> * 8 + 2);
    }
    if ((rights & CastleShort) && !(CastleShortEmptyMask & all) &&
        !(CastleShortSafeMask & unsafe)) {
      moves.append(
        CASTLE_SHORT, RelativeRank<Player, 0> * 8 + 4, RelativeRank<Player, 0> * 8 + 6);
    }
  }
  return false;
}

bool generateMoves(const Position& p, MoveList& moves)
{
  if (p.turn() == WHT) {
    return generateMoves<WHT>(p, moves);
  }
  else if (p.turn() == BLK) {
    return generateMoves<BLK>(p, moves);
  }
  return false;
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

Response Response::none()
{
  return {std::nullopt, Conclusion::NONE};
}

bool Response::isNone() const
{
  return !mMove.has_value() && mConclusion == Conclusion::NONE;
}

}  // namespace potato

namespace std {

using namespace potato;
static constexpr auto coord = SquareCoord;

std::ostream& operator<<(std::ostream& os, const Move& m)
{
  os << m.algebraic();
  return os;
}

}  // namespace std
