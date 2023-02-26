#include <Move.h>

namespace potato {

static float eval(Position& p)
{
  return float(p.material());
}

Move bestMove(Position& position)
{
  MoveList                            moves;
  StaticVector<float, MoveList::Size> evals;
  generateMoves(position, moves);
  evals.resize(moves.size(), 0);
  // TODO: Parallelize this later.
  std::transform(moves.begin(), moves.end(), evals.begin(), [&](Move m) {
    m.commit(position);
    float e = eval(position);
    m.revert(position);
    return e;
  });
  auto   best = position.turn() == WHT ? std::max_element(evals.begin(), evals.end())
                                       : std::min_element(evals.begin(), evals.end());
  size_t idx  = size_t(std::distance(evals.begin(), best));
  return moves[idx];
}

}  // namespace potato
