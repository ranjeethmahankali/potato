#include <Board.h>
#include <catch2/catch_all.hpp>
#include <iostream>

using namespace potato;

TEST_CASE("Moves from the start", "[moves][starting]")
{
  static constexpr size_t                     NMoves         = 3;
  static constexpr std::array<size_t, NMoves> sExpectedMoves = {{20, 400, 8902}};
  std::vector<Board>                          src(1);
  std::vector<Board>                          dst;
  std::array<size_t, NMoves>                  steps;
  for (size_t i = 0; i < NMoves; ++i) {
    dst.clear();
    uint8_t turn = (i % 2) ? Piece::BLK : Piece::WHT;
    for (const Board& b : src) {
      b.genMoves(dst, turn);
    }
    steps[i] = dst.size();
    std::swap(src, dst);
  }
  REQUIRE(steps == sExpectedMoves);
}
