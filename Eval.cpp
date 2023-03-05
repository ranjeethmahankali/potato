#include <Move.h>
#include <algorithm>
#include <climits>

namespace potato {

static int staticEval(Position& p)
{
  // TODO: Piece square tables: https://www.chessprogramming.org/Piece-Square_Tables
  // TODO: Pawn structures: https://www.chessprogramming.org/Pawn_Structure
  // TODO: Mobility: https://www.chessprogramming.org/Mobility
  // TODO: King Safety / King Tropism: https://www.chessprogramming.org/King_Safety
  // MAYBE: Connectivity: https://www.chessprogramming.org/Connectivity
  return p.material();
}

static int evalMove(Move m, const Position& position)
{
  int score = std::abs(
    MaterialValue[position.piece(m.to())]);  // Reward for the piece being captured.
  MoveType type   = m.type();
  auto     rights = uint8_t(position.castlingRights());
  int      qrok   = -1;
  int      krok   = -1;
  if (position.turn() == WHT) {
    rights >>= 2;
    qrok = 56;
    krok = 63;
  }
  else {
    rights &= 0b11;
    qrok = 0;
    krok = 7;
  }
  if ((rights && (type == MV_KNG))) {
    score -= 8;
  }
  else if (((rights & 1) && type == MV_ROK && m.from() == qrok) ||
           ((rights & 2) && type == MV_ROK && m.from() == krok)) {
    score -= 7;  // Penalty for losing castling rights.
  }
  if ((type == CASTLE_LONG) || (type == CASTLE_SHORT)) {
    score += 8;
  }
  return score;
}

static void sortMoves(MoveList& moves, const Position& position)
{
  StaticVector<std::pair<Move, int>, MoveList::Size> mpairs;
  std::transform(moves.begin(), moves.end(), std::back_inserter(mpairs), [&](Move m) {
    return std::make_pair(m, evalMove(m, position));
  });
  std::sort(mpairs.begin(),
            mpairs.end(),
            [](const std::pair<Move, int>& a, const std::pair<Move, int>& b) {
              return a.second > b.second;
            });
  std::transform(
    mpairs.begin(), mpairs.end(), moves.begin(), [](const std::pair<Move, int>& mpair) {
      return mpair.first;
    });
}

int maximize(Position& position,
             int       depth,
             Response& move,
             int       alpha = INT_MIN,
             int       beta  = INT_MAX);

int minimize(Position& position,
             int       depth,
             Response& move,
             int       alpha = INT_MIN,
             int       beta  = INT_MAX)
{
  if (depth == 0) {
    return staticEval(position);
  }
  MoveList moves;
  bool     inCheck = generateMoves(position, moves);
  sortMoves(moves, position);
  int best = INT_MAX;
  for (Move m : moves) {
    m.commit(position);
    Response next;
    int      eval = maximize(position, depth - 1, next, alpha, beta);
    m.revert(position);
    beta = std::min(beta, eval);
    if (eval < best) {
      best = eval;
      move = Response {m, Conclusion::NONE};
    }
    if (beta <= alpha) {
      break;
    }
  }
  if (moves.empty()) {
    if (inCheck) {
      move = {std::nullopt, Conclusion::CHECKMATE};
      return 100;
    }
    else {
      move = {std::nullopt, Conclusion::STALEMATE};
      return 0;
    }
  }
  return best;
}

int maximize(Position& position, int depth, Response& move, int alpha, int beta)
{
  if (depth == 0) {
    return staticEval(position);
  }
  MoveList moves;
  bool     inCheck = generateMoves(position, moves);
  sortMoves(moves, position);
  int best = INT_MIN;
  for (Move m : moves) {
    m.commit(position);
    Response next;
    int      eval = minimize(position, depth - 1, next, alpha, beta);
    m.revert(position);
    alpha = std::max(alpha, eval);
    if (eval > best) {
      best = eval;
      move = {m, Conclusion::NONE};
    }
    if (beta <= alpha) {
      break;
    }
  }
  if (moves.empty()) {
    if (inCheck) {
      move = {std::nullopt, Conclusion::CHECKMATE};
      return -100;
    }
    else {
      move = {std::nullopt, Conclusion::STALEMATE};
      return 0;
    }
  }
  return best;
}

Response minimax(Position& position, int alpha = INT_MIN, int beta = INT_MAX)
{
  Response             move;
  static constexpr int Depth = 8;
  if (position.turn() == WHT) {
    maximize(position, Depth, move, alpha, beta);
  }
  else {
    minimize(position, Depth, move, alpha, beta);
  }
  return move;
}

Response bestMove(Position& position)
{
  return minimax(position);
}

}  // namespace potato
