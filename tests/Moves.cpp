#include <Position.h>
#include <catch2/catch_all.hpp>
#include <iostream>

using namespace potato;

TEST_CASE("Moves from the start", "[moves][starting]")
{
  static constexpr size_t                     NMoves         = 3;
  static constexpr std::array<size_t, NMoves> sExpectedMoves = {{20, 400, 8902}};
  std::vector<Position>                       src(1);
  std::vector<Position>                       dst;
  std::vector<Position>                       temp;
  std::array<size_t, NMoves>                  steps;
  for (size_t i = 0; i < NMoves; ++i) {
    dst.clear();
    uint8_t turn = (i % 2) ? Piece::BLK : Piece::WHT;
    for (const Position& b : src) {
      temp.clear();
      // if (i == 2) {
      //   std::cout << "here\n";
      // }
      b.genMoves(temp, turn);
      if (i == 2) {
        size_t counter = 0;
        // for (const auto& b : temp) {
        //   std::cout << counter++ << std::endl;
        //   std::cout << b << std::endl;
        // }
        std::cout << temp.size() << std::endl;
      }
      std::copy(temp.begin(), temp.end(), std::back_inserter(dst));
    }
    steps[i] = dst.size();
    std::swap(src, dst);
  }
  REQUIRE(steps == sExpectedMoves);
}
