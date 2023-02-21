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

TEST_CASE("Move generation test 1", "[move-gen][case-1]")
{
  doPerftTest("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
              6,
              {{20, 400, 8902, 197281, 4865609, 119060324}});
}

TEST_CASE("Move generation test 2", "[move-gen][case-2]")
{
  doPerftTest("r1bqkb1r/ppp2ppp/2n5/1B1pp3/3Pn3/5N2/PPP2PPP/RNBQ1RK1 b kq - 1 6",
              6,
              {{38, 1357, 51428, 1842992, 71427276, 2584961600}});
}

TEST_CASE("Move generation test 3", "[move-gen][case-3]")
{
  doPerftTest("r1bqk2r/ppppbppp/5n2/4n3/2B1P3/3P4/PPP2PPP/RNB1K1NR w KQkq - 0 6",
              6,
              {{33, 1053, 32371, 1056722, 32819055, 1096036679}});
}

TEST_CASE("Move generation test 4", "[move-gen][case-4]")
{
  doPerftTest("5rk1/5ppp/p1pqpn2/1r1p4/3P1N2/1PP2Q2/P4PPP/R4RK1 w - - 1 18",
              6,
              {{34, 1212, 41471, 1467118, 50367135, 1776221475}});
}

TEST_CASE("Move generation test 5", "[move-gen][case-5]")
{
  doPerftTest("2r5/8/p6p/1p2R3/5k2/2P5/PP3P1P/4R1K1 b - - 0 30",
              6,
              {{17, 497, 7303, 212706, 3163842, 90738905}});
}

TEST_CASE("Move generation test 6", "[move-gen][case-6]")
{
  doPerftTest("1n2n3/5b1p/pP1p2kp/2q3P1/4pKp1/rP2r3/2N5/1B6 w - - 0 1",
              6,
              {{11, 491, 5328, 225328, 2555688, 105197834}});
}

TEST_CASE("Move generation test 7", "[move-gen][case-7]")
{
  doPerftTest("8/p6b/7r/4k3/R7/6p1/2N5/1K6 w - - 0 1",
              6,
              {{17, 398, 6451, 147056, 2573546, 59282346}});
}

TEST_CASE("Move generation test 8", "[move-gen][case-8]")
{
  doPerftTest("3k4/8/8/7K/8/5r2/8/8 w - - 0 1", 6, {{5, 95, 458, 8481, 42917, 799980}});
}

TEST_CASE("Move generation test 9", "[move-gen][case-9]")
{
  doPerftTest("R1Nn4/1R1QPp2/3P1P1p/pr3Npr/p2n1Ppq/2Pk1Bp1/1P1b1p1B/1b1K4 w - - 0 1",
              6,
              {{45, 1569, 63965, 2180013, 87383660, 2943164039}});
}

TEST_CASE("Move generation test 10", "[move-gen][case-10]")
{
  doPerftTest("8/6P1/2B4Q/N2p2P1/p1nkP3/Rp1b3P/1pPb2KP/6n1 w - - 0 1",
              6,
              {{37, 1084, 37639, 1071195, 37775427, 1059075718}});
}

TEST_CASE("Move generation test 11", "[move-gen][case-11]")
{
  doPerftTest("8/5B2/1Rp1rN1P/3pPQ2/5pp1/1PBnb1k1/2r1n2R/3K4 w - - 0 1",
              6,
              {{45, 1274, 48000, 1351193, 51419051, 1456329059}});
}

TEST_CASE("Move generation test 12", "[move-gen][case-12]")
{
  doPerftTest(
    "2k5/5K2/8/2n5/R7/2P5/8/8 w - - 0 1", 6, {{22, 257, 4950, 51658, 955161, 9876262}});
}

TEST_CASE("Move generation test 13", "[move-gen][case-13]")
{
  doPerftTest(
    "7K/P7/8/8/1n5p/3k4/8/8 w - - 0 1", 6, {{7, 96, 1112, 14487, 216507, 2604966}});
}

TEST_CASE("Move generation test 14", "[move-gen][case-14]")
{
  doPerftTest("8/3k4/7K/8/8/8/8/6b1 w - - 0 1", 6, {{5, 75, 475, 7156, 39960, 610271}});
}

TEST_CASE("Move generation test 15", "[move-gen][case-15]")
{
  doPerftTest(
    "8/3k4/8/1N6/8/8/8/QK6 w - - 0 1", 6, {{24, 123, 3307, 14001, 407791, 1804599}});
}

TEST_CASE("Move generation test 16", "[move-gen][case-16]")
{
  doPerftTest("k7/n3r3/6QP/5P1p/5p1p/K1pPb3/1RN4N/8 w - - 0 1",
              6,
              {{40, 809, 28447, 607528, 20865139, 450840527}});
}

TEST_CASE("Move generation test 17", "[move-gen][case-17]")
{
  doPerftTest("8/8/1k6/8/6K1/8/8/8 w - - 0 1", 6, {{8, 64, 440, 2995, 21700, 147458}});
}

TEST_CASE("Move generation test 18", "[move-gen][case-18]")
{
  doPerftTest(
    "8/8/8/4K3/8/8/3bk3/8 w - - 0 1", 6, {{7, 107, 701, 10557, 70227, 1044208}});
}

TEST_CASE("Move generation test 19", "[move-gen][case-19]")
{
  doPerftTest("8/8/8/8/8/8/K1kP1p2/8 w - - 0 1", 6, {{4, 36, 144, 1627, 7533, 103947}});
}

TEST_CASE("Move generation test 20", "[move-gen][case-20]")
{
  doPerftTest(
    "8/8/8/2k4N/8/1r6/5p2/5K2 w - - 0 1", 6, {{7, 159, 1356, 27530, 242599, 4986912}});
}

TEST_CASE("Move generation test 21", "[move-gen][case-21]")
{
  doPerftTest(
    "6b1/7q/8/2K5/1P6/5P2/3k4/8 w - - 0 1", 6, {{7, 235, 1331, 44431, 252433, 8331669}});
}

TEST_CASE("Move generation test 22", "[move-gen][case-22]")
{
  doPerftTest("8/2K1n3/3P4/6k1/8/p7/4P3/5R2 w - - 0 1",
              6,
              {{23, 271, 5491, 63225, 1261844, 14718791}});
}

TEST_CASE("Move generation test 23", "[move-gen][case-23]")
{
  doPerftTest("8/4r3/8/4p3/8/1p2P3/6P1/1N1k1K2 w - - 0 1",
              6,
              {{8, 96, 831, 12453, 119134, 1926544}});
}

TEST_CASE("Move generation test 24", "[move-gen][case-24]")
{
  doPerftTest("8/2n5/3p4/3r4/KN6/5B1p/1k5n/8 w - - 0 1",
              6,
              {{14, 291, 4054, 87619, 1230824, 27248921}});
}

TEST_CASE("Move generation test 25", "[move-gen][case-25]")
{
  doPerftTest("8/5k2/1P6/7p/p7/B1P2p2/R5N1/1K6 w - - 0 1",
              6,
              {{23, 207, 4858, 43599, 1023025, 9444084}});
}

TEST_CASE("Move generation test 26", "[move-gen][case-26]")
{
  doPerftTest("b4bq1/1p3r2/2p1P3/P2k4/8/8/4P3/1K6 w - - 0 1",
              6,
              {{10, 324, 3101, 105300, 1034365, 35775794}});
}

TEST_CASE("Move generation test 27", "[move-gen][case-27]")
{
  doPerftTest("3Q4/8/7p/n5N1/P1b1K3/q5p1/1P2k3/B7 w - - 0 1",
              6,
              {{31, 884, 22176, 629919, 15119886, 430812288}});
}

TEST_CASE("Move generation test 28", "[move-gen][case-28]")
{
  doPerftTest("Q5R1/1p4P1/4K2p/1r6/1q6/8/PPP5/3kN3 w - - 0 1",
              6,
              {{28, 837, 19186, 605839, 15318551, 477296703}});
}

TEST_CASE("Move generation test 29", "[move-gen][case-29]")
{
  doPerftTest("n7/R4p2/3Kpk2/3P4/4Q3/1Br3P1/5P2/4b2b w - - 0 1",
              6,
              {{43, 809, 29469, 622040, 20890188, 471809369}});
}

TEST_CASE("Move generation test 30", "[move-gen][case-30]")
{
  doPerftTest("8/7R/4P1pr/8/n3pB2/k4r1P/2P3p1/n1K1Q3 w - - 0 1",
              6,
              {{40, 978, 32796, 854403, 27019441, 731173852}});
}

TEST_CASE("Move generation test 31", "[move-gen][case-31]")
{
  doPerftTest("q3R1B1/2bk2P1/3p3p/QP2p3/3P3K/p4b2/8/4R3 w - - 0 1",
              6,
              {{41, 1117, 42164, 1069525, 41132104, 1028636700}});
}

TEST_CASE("Move generation test 32", "[move-gen][case-32]")
{
  doPerftTest("5B2/2k2q2/B7/3PppR1/1P1pP2K/P1n1P3/P7/3N4 w - - 0 1",
              6,
              {{31, 809, 22432, 576953, 15497096, 393452066}});
}
