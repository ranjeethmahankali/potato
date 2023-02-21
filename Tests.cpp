#define CATCH_CONFIG_MAIN
#include <Move.h>
#include <algorithm>
#include <bit>
#include <catch.hpp>
#include <catch2/catch_all.hpp>
#include <iostream>
#include <stack>

using namespace potato;

static void push(const MoveList&                      mlist,
                 std::stack<std::pair<Move, size_t>>& moves,
                 size_t                               depth)
{
  for (const Move& m : mlist) {
    moves.emplace(m, depth);
  }
}

static void doPerftTest(const std::string&      fenstr,
                        size_t                  depth,
                        std::span<const size_t> expected)
{
  REQUIRE(expected.size() == depth);
  std::vector<size_t>                 actual(depth, 0);
  Position                            p = Position::fromFen(fenstr);
  MoveList                            mlist;
  std::stack<std::pair<Move, size_t>> moves;
  std::stack<Move>                    current;
  std::stack<Position>                positions;
  positions.push(p);
  do {
    if (!moves.empty()) {
      auto mvd = moves.top();
      moves.pop();
      while (mvd.second <= current.size()) {
        current.top().revert(p);
        current.pop();
        positions.pop();
        REQUIRE(p == positions.top());
      }
      mvd.first.commit(p);
      positions.push(p);
      current.push(mvd.first);
    }
    if (current.size() < depth) {
      generateMoves(p, mlist);
      actual[current.size()] += mlist.size();
      if (current.size() + 1 < depth) {
        push(mlist, moves, current.size() + 1);
      }
      mlist.clear();
    }
  } while (!moves.empty());
  REQUIRE(actual.size() == expected.size());
  for (size_t i = 0; i < actual.size(); ++i) {
    REQUIRE(actual[i] == expected[i]);
  }
}

TEST_CASE("Perft Results 1", "[perft][starting][case-1]")
{
  doPerftTest("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
              6,
              {{20, 400, 8902, 197281, 4865609, 119060324}});
}

TEST_CASE("Perft Results 2", "[perft][case-2]")
{
  doPerftTest("r1bqkb1r/ppp2ppp/2n5/1B1pp3/3Pn3/5N2/PPP2PPP/RNBQ1RK1 b kq - 1 6",
              6,
              {{38, 1357, 51428, 1842992, 71427276, 2584961600}});
}

TEST_CASE("Perft Results 3", "[perft][case-3]")
{
  doPerftTest("r1bqk2r/ppppbppp/5n2/4n3/2B1P3/3P4/PPP2PPP/RNB1K1NR w KQkq - 0 6",
              6,
              {{33, 1053, 32371, 1056722, 32819055, 1096036679}});
}

TEST_CASE("Perft Results 4", "[perft][case-4]")
{
  doPerftTest("5rk1/5ppp/p1pqpn2/1r1p4/3P1N2/1PP2Q2/P4PPP/R4RK1 w - - 1 18",
              6,
              {{34, 1212, 41471, 1467118, 50367135, 1776221475}});
}

TEST_CASE("Perft Results 5", "[perft][case-5]")
{
  doPerftTest("2r5/8/p6p/1p2R3/5k2/2P5/PP3P1P/4R1K1 b - - 0 30",
              6,
              {{17, 497, 7303, 212706, 3163842, 90738905}});
}

TEST_CASE("Fen Consistency", "[fen][consistency]")
{
  SECTION("Case 1")
  {
    Position p =
      Position::fromFen("rnbqkbnr/1ppppppp/p7/P7/8/8/1PPPPPPP/RNBQKBNR b KQkq - 0 2");
    Move(MvDoublePush<BLK> {B7}).commit(p);
    REQUIRE(p.fen() == "rnbqkbnr/2pppppp/p7/Pp6/8/8/1PPPPPPP/RNBQKBNR w KQkq b6 0 3");
  }
  SECTION("Case 2")
  {
    Position p =
      Position::fromFen("rnbqkbnr/pppp1ppp/4p3/8/5P2/5N2/PPPPP1PP/RNBQKB1R b KQkq - 1 2");
    Move(MvPiece {D8, H4}).commit(p);
    REQUIRE(p.fen() == "rnb1kbnr/pppp1ppp/4p3/8/5P1q/5N2/PPPPP1PP/RNBQKB1R w KQkq - 2 3");
  }
  SECTION("Case 3")
  {
    Position p = Position::fromFen(
      "r1bqk2r/ppp2ppp/2n5/1B1pp3/3Pn3/b1N2N2/PPP2PPP/R1BQ1RK1 b kq - 3 7");
    Move(MvPiece {E8, D7}).commit(p);
    REQUIRE(p.fen() ==
            "r1bq3r/pppk1ppp/2n5/1B1pp3/3Pn3/b1N2N2/PPP2PPP/R1BQ1RK1 w - - 4 8");
  }
  SECTION("Case 4")
  {
    Position p = Position::fromFen(
      "r1bqk2r/ppppbp1p/8/3nB1p1/2B1P3/3P4/PPP2PPP/RN2K1NR w KQkq - 0 8");
    Move(MvPiece {E5, H8}).commit(p);
    REQUIRE(p.fen() == "r1bqk2B/ppppbp1p/8/3n2p1/2B1P3/3P4/PPP2PPP/RN2K1NR b KQq - 0 8");
  }
}

TEST_CASE("Perft From Starting Position", "[perft][debug]")
{
  Position p = Position::fromFen("2r5/8/p6p/1p2R3/8/2P2k2/PP3P1P/3R2K1 b - - 2 31");
  std::cout << p << std::endl;
  perft(p, 1);
}

TEST_CASE("Loading from FEN string", "[fen][parsing][generation]")
{
  std::string fenstr =
    "2R1N2N/P4K1P/qr5p/1bP1pPRP/1P1pPp1B/ppb2n2/2rppnPQ/2k4B w - - 0 1";
  Position pfen     = Position::fromFen(fenstr);
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
  std::cout << pfen.fen() << std::endl;
  REQUIRE(pfen.fen() == fenstr);
}

TEST_CASE("Slider move bitboards", "[slider][moves][bitboards]")
{
  Position p = Position::fromFen("Q1n5/5nb1/6KP/1P2P1p1/1R2p3/q7/b1k4p/1N6 w - - 0 1");
  BitBoard black    = getAllBoards<BLK>(p);
  BitBoard white    = getAllBoards<WHT>(p);
  BitBoard notblack = ~black;
  BitBoard notwhite = ~white;
  BitBoard all      = black | white;

  SECTION("Black Bishop")
  {
    BitBoard                                 bbsh           = p.board(B_BSH);
    static constexpr std::array<BitBoard, 2> sExpectedMoves = {
      {278921376, 144117404414246912}};
    REQUIRE(std::popcount(bbsh) == 2);
    // writeBoard(bbsh, std::cout); // To check manually.
    std::array<BitBoard, 2> actual;
    BitBoard*               dst = actual.data();
    while (bbsh) {
      BitBoard moves = bishopMoves(pop(bbsh), all) & notblack;
      // writeBoard(moves, std::cout); // To check manually.
      *(dst++) = moves;
    }
    REQUIRE(sExpectedMoves == actual);
  }

  SECTION("White Rook")
  {
    auto wrok = p.board(W_ROK);
    REQUIRE(std::popcount(wrok) == 1);
    // writeBoard(wrok, std::cout);  // To check manually
    while (wrok) {
      auto moves = rookMoves(pop(wrok), all) & notwhite;
      // writeBoard(moves, std::cout);  // To check manually
      REQUIRE(moves == 565273530728448);
    }
  }

  SECTION("White Queen")
  {
    auto wqen = p.board(W_QEN);
    REQUIRE(std::popcount(wqen) == 1);
    // writeBoard(wqen, std::cout);  // To check manually
    while (wqen) {
      auto moves = queenMoves(pop(wqen), all) & notwhite;
      // writeBoard(moves, std::cout);  // To check manually
      REQUIRE(moves == 1172677395206);
    }
  }

  SECTION("Black Queen")
  {
    auto bqen = p.board(B_QEN);
    REQUIRE(std::popcount(bqen) == 1);
    // writeBoard(bqen, std::cout);  // To check manually
    while (bqen) {
      auto moves = queenMoves(pop(bqen), all) & notblack;
      // writeBoard(moves, std::cout);  // To check manually
      REQUIRE(moves == 289072614960333057);
    }
  }
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
