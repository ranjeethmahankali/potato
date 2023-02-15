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

TEST_CASE("Loading from FEN string", "[fen][parsing]")
{
  Position pfen = Position::fromFen(
    "2R1N2N/P4K1P/qr5p/1bP1pPRP/1P1pPp1B/ppb2n2/2rppnPQ/2k4B w - - 0 1");
  Position expected = Position::empty();
  expected
    .put({{{C1, B_KNG}, {H1, W_BSH}, {C2, B_ROK}, {D2, B_PWN}, {E2, B_PWN}, {F2, B_HRS},
           {G2, W_PWN}, {H2, W_QEN}, {A3, B_PWN}, {B3, B_PWN}, {C3, B_BSH}, {F3, B_HRS},
           {B4, W_PWN}, {D4, B_PWN}, {E4, W_PWN}, {F4, B_PWN}, {H4, W_BSH}, {B5, B_BSH},
           {C5, W_PWN}, {E5, B_PWN}, {F5, W_PWN}, {G5, W_ROK}, {H5, W_PWN}, {A6, B_QEN},
           {B6, B_ROK}, {H6, B_PWN}, {A7, W_PWN}, {F7, W_KNG}, {H7, W_PWN}, {C8, W_ROK},
           {E8, W_HRS}, {H8, W_HRS}}})
    .setCastlingRights(Castle(0));
  expected.setEnpassantSq(-1);
  expected.setTurn(WHT);
  REQUIRE(pfen.valid());
  REQUIRE(expected.valid());
  REQUIRE(pfen == expected);
}

TEST_CASE("Slider attack squares", "[slider][attack][bitboards]")
{
  Position p = Position::fromFen("Q1n5/5nb1/6KP/1P2P1p1/1R2p3/q7/b1k4p/1N6 w - - 0 1");
  // TODO: Incomplete.
  REQUIRE(false);
}

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
