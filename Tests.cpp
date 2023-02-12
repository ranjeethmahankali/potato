#define CATCH_CONFIG_MAIN
#include <Position.h>
#include <catch.hpp>
#include <catch2/catch_all.hpp>
#include <iostream>

using namespace potato;

// TEST_CASE("Moves from the start", "[moves][starting]")
// {
//   static constexpr size_t                     NMoves         = 3;
//   static constexpr std::array<size_t, NMoves> sExpectedMoves = {{20, 400, 8902}};
//   std::vector<Position>                       src(1);
//   std::vector<Position>                       dst;
//   std::vector<Position>                       temp;
//   std::array<size_t, NMoves>                  steps;
//   for (size_t i = 0; i < NMoves; ++i) {
//     dst.clear();
//     Color turn = (i % 2) ? Color::BLK : Color::WHT;
//     for (const Position& b : src) {
//       temp.clear();
//       // if (i == 2) {
//       //   std::cout << "here\n";
//       // }
//       // b.genMoves(temp, turn);
//       if (i == 2) {
//         size_t counter = 0;
//         // for (const auto& b : temp) {
//         //   std::cout << counter++ << std::endl;
//         //   std::cout << b << std::endl;
//         // }
//         std::cout << temp.size() << std::endl;
//       }
//       // std::copy(temp.begin(), temp.end(), std::back_inserter(dst));
//     }
//     steps[i] = dst.size();
//     std::swap(src, dst);
//   }
//   REQUIRE(steps == sExpectedMoves);
// }

TEST_CASE("Zobrist Hash Updates", "[zobrist][hash][incremental][update]")
{
  SECTION("Reversible put / remove")
  {
    Position p;
    REQUIRE(p.valid());
    uint64_t h1 = p.hash();
    p.remove(0);
    REQUIRE(p.valid());
    REQUIRE(p.hash() != h1);
    p.put(0, Piece::B_ROK);
    REQUIRE(p.valid());
    REQUIRE(p.hash() == h1);
  }

  SECTION("Reversible move")
  {
    Position p;
    uint64_t h1 = p.hash();
    p.move(8, 24);
    REQUIRE(p.valid());
    REQUIRE(h1 != p.hash());
    p.move(24, 8);
    REQUIRE(p.valid());
    REQUIRE(h1 == p.hash());
  }

  SECTION("Move cycle")
  {
    Position p;
    p.clear();
    REQUIRE(p.valid());
    p.put({2, 2}, Piece::B_ROK);
    REQUIRE(p.valid());
    uint64_t h1 = p.hash();
    p.move({2, 2}, {2, 4});
    REQUIRE(p.valid());
    uint64_t h2 = p.hash();
    REQUIRE(h1 != h2);
    p.move({2, 4}, {4, 4});
    REQUIRE(p.valid());
    uint64_t h3 = p.hash();
    REQUIRE(h1 != h3);
    REQUIRE(h2 != h3);
    p.move({4, 4}, {4, 2});
    REQUIRE(p.valid());
    uint64_t h4 = p.hash();
    REQUIRE(h1 != h4);
    REQUIRE(h2 != h4);
    REQUIRE(h3 != h4);
    p.move({4, 2}, {2, 2});
    REQUIRE(p.valid());
    REQUIRE(h1 == p.hash());
  }
}
