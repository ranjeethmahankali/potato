#include <Move.h>

namespace potato {

static float eval(Position& p)
{
  return float(p.material());
}

Response bestMove(Position& position)
{
  MoveList                            moves;
  StaticVector<float, MoveList::Size> evals;
  bool                                inCheck = generateMoves(position, moves);
  if (!moves.empty()) {
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
    return {moves[idx], Conclusion::NONE};
  }
  else if (inCheck) {
    return {std::nullopt, Conclusion::CHECKMATE};
  }
  else {
    return {std::nullopt, Conclusion::STALEMATE};
  }
}

}  // namespace potato
