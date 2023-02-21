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

TEST_CASE("Move generation test 33", "[move-gen][case-33]")
{
  doPerftTest("rnbqkb1r/p1p2ppp/1p2pn2/3p4/2P5/P1N1P3/1P1P1PPP/R1BQKBNR w KQkq - 0 5",
              6,
              {{36, 1194, 41706, 1416130, 48927843, 1695942724}});
}

TEST_CASE("Move generation test 34", "[move-gen][case-34]")
{
  doPerftTest("r1bqkb1r/pp1p1ppp/2n1pn2/2p5/4P3/N1P2N2/PP1P1PPP/R1BQKB1R w KQkq - 2 5",
              6,
              {{31, 986, 31965, 1039656, 34650033, 1150572351}});
}

TEST_CASE("Move generation test 35", "[move-gen][case-35]")
{
  doPerftTest("rnbqkb1r/ppp2p1p/3p1np1/4p3/2P5/2NPP3/PP3PPP/R1BQKBNR w KQkq - 0 5",
              6,
              {{35, 1124, 38460, 1255199, 42621450, 1417116896}});
}

TEST_CASE("Move generation test 36", "[move-gen][case-36]")
{
  doPerftTest("rnbqkb1r/1p2pppp/2p2n2/p2p4/2PP4/5N2/PP1NPPPP/R1BQKB1R w KQkq - 0 5",
              6,
              {{24, 740, 19613, 614147, 17679277, 570316490}});
}

TEST_CASE("Move generation test 37", "[move-gen][case-37]")
{
  doPerftTest("rnbqkbnr/p2p1p1p/1p2p1p1/2p5/8/1P1P1NP1/P1P1PP1P/RNBQKB1R w KQkq - 0 5",
              6,
              {{32, 922, 28884, 855057, 26884650, 823209297}});
}

TEST_CASE("Move generation test 38", "[move-gen][case-38]")
{
  doPerftTest("rnbqkbnr/1p1p1p1p/p3p1p1/2p5/2B1P3/3P3P/PPP2PP1/RNBQK1NR w KQkq - 0 5",
              6,
              {{36, 1010, 35944, 1021677, 36717409, 1075053157}});
}

TEST_CASE("Move generation test 39", "[move-gen][case-39]")
{
  doPerftTest("rnbqk2r/p1ppppbp/1p3np1/8/3P4/1P3N2/PBP1PPPP/RN1QKB1R w KQkq - 0 5",
              6,
              {{28, 753, 21943, 618608, 18819585, 550948819}});
}

TEST_CASE("Move generation test 40", "[move-gen][case-40]")
{
  doPerftTest("r1bqkb1r/pp1p1ppp/2n1pn2/2p5/4P3/2P2N2/PPQP1PPP/RNB1KB1R w KQkq - 0 5",
              6,
              {{30, 956, 30155, 985528, 31961366, 1067938803}});
}

TEST_CASE("Move generation test 41", "[move-gen][case-41]")
{
  doPerftTest("8/1p2r3/8/5R2/1p1K4/6pP/4P3/1k6 w - - 0 1",
              6,
              {{21, 390, 6870, 128989, 2199851, 41283799}});
}

TEST_CASE("Move generation test 42", "[move-gen][case-42]")
{
  doPerftTest("8/7P/k7/1p3RP1/3P2p1/8/7P/3K4 w - - 0 1",
              6,
              {{24, 139, 3492, 22968, 589144, 3974560}});
}

TEST_CASE("Move generation test 43", "[move-gen][case-43]")
{
  doPerftTest("8/2q5/5p2/2PP1k2/7P/2R4b/8/4K3 w - - 0 1",
              6,
              {{17, 422, 6756, 158158, 2528580, 60638512}});
}

TEST_CASE("Move generation test 44", "[move-gen][case-44]")
{
  doPerftTest("1n6/3rPb2/8/8/5K2/1N6/1P3k1B/8 w - - 0 1",
              6,
              {{17, 386, 7213, 169103, 3455180, 82609710}});
}

TEST_CASE("Move generation test 45", "[move-gen][case-45]")
{
  doPerftTest("4bb2/7K/2P5/8/R5P1/7p/8/4qk2 w - - 0 1",
              6,
              {{16, 494, 7011, 231298, 3383251, 114578891}});
}

TEST_CASE("Move generation test 46", "[move-gen][case-46]")
{
  doPerftTest("5N2/8/3RK3/p7/3p4/3k3p/8/Q6n w - - 0 1",
              6,
              {{31, 241, 8082, 64386, 2239157, 19050888}});
}

TEST_CASE("Move generation test 47", "[move-gen][case-47]")
{
  doPerftTest("8/4n1p1/8/6r1/1k3K2/N3p3/p4p2/8 w - - 0 1",
              6,
              {{8, 235, 1990, 61774, 542486, 17120519}});
}

TEST_CASE("Move generation test 48", "[move-gen][case-48]")
{
  doPerftTest("8/1K2p3/2P5/8/7b/Pk6/4QBn1/8 w - - 0 1",
              6,
              {{37, 382, 13136, 161574, 5298027, 72041315}});
}

TEST_CASE("Move generation test 49", "[move-gen][case-49]")
{
  doPerftTest("8/3P4/6p1/P3K3/1p6/6k1/3Pp3/R6R w - - 0 1",
              6,
              {{35, 349, 9854, 99642, 2788061, 31316954}});
}

TEST_CASE("Move generation test 50", "[move-gen][case-50]")
{
  doPerftTest("1k6/6p1/b1p5/1r5p/8/6P1/7B/2N1K3 w - - 0 1",
              6,
              {{11, 232, 2491, 55230, 640388, 14304260}});
}

TEST_CASE("Move generation test 51", "[move-gen][case-51]")
{
  doPerftTest("8/kPK5/4bP2/8/4p1r1/4P3/3p2P1/8 w - - 0 1",
              6,
              {{9, 162, 1788, 37035, 467277, 10221239}});
}

TEST_CASE("Move generation test 52", "[move-gen][case-52]")
{
  doPerftTest("2q5/2k5/6p1/8/K1b1Q3/4R1PP/4R3/8 w - - 0 1",
              6,
              {{39, 958, 31631, 751420, 25429976, 612441588}});
}

TEST_CASE("Move generation test 53", "[move-gen][case-53]")
{
  doPerftTest("7R/7p/k2PB3/8/3K4/3pQ3/5p1p/8 w - - 0 1",
              6,
              {{41, 571, 24347, 379626, 15434752, 266629424}});
}

TEST_CASE("Move generation test 54", "[move-gen][case-54]")
{
  doPerftTest("8/5r2/6bk/1P4p1/3n4/K3p3/5p2/6b1 w - - 0 1",
              6,
              {{5, 185, 948, 34977, 177163, 6552521}});
}

TEST_CASE("Move generation test 55", "[move-gen][case-55]")
{
  doPerftTest("1b6/2q5/4r3/8/5p2/k5p1/1p1K2b1/5N2 w - - 0 1",
              6,
              {{5, 261, 1674, 80576, 521710, 24861687}});
}

TEST_CASE("Move generation test 56", "[move-gen][case-56]")
{
  doPerftTest("5k1B/1b1P1p2/3R4/p6R/8/8/5K1p/8 w - - 0 1",
              6,
              {{40, 645, 23736, 379226, 13463681, 224292679}});
}

TEST_CASE("Move generation test 57", "[move-gen][case-57]")
{
  doPerftTest("2k5/4p3/8/p6r/3p1RPp/8/6K1/2b3N1 w - - 0 1",
              6,
              {{20, 453, 8510, 190558, 3590535, 80120413}});
}

TEST_CASE("Move generation test 58", "[move-gen][case-58]")
{
  doPerftTest("8/6pp/8/4B1R1/2P1p2K/1k3b2/5nN1/8 w - - 0 1",
              6,
              {{23, 465, 11389, 218423, 5328494, 99475393}});
}

TEST_CASE("Move generation test 59", "[move-gen][case-59]")
{
  doPerftTest("1NK1k3/6r1/8/2p4P/1P5P/p7/4p3/3B4 w - - 0 1",
              6,
              {{10, 231, 2957, 64410, 925830, 20553642}});
}

TEST_CASE("Move generation test 60", "[move-gen][case-60]")
{
  doPerftTest("8/3p1R1p/4k3/2p5/4P3/6R1/2p2K1B/1r6 w - - 0 1",
              6,
              {{29, 644, 19031, 373763, 10698975, 211293971}});
}

TEST_CASE("Move generation test 61", "[move-gen][case-61]")
{
  doPerftTest("8/1P1K4/1N5b/7r/3p2B1/4r2P/4p3/5k2 w - - 0 1",
              6,
              {{20, 629, 13060, 418352, 9103101, 298368687}});
}

TEST_CASE("Move generation test 62", "[move-gen][case-62]")
{
  doPerftTest("8/NPn1Ppp1/8/7Q/3p4/2P4K/8/4k3 w - - 0 1",
              6,
              {{35, 460, 16032, 202167, 7006851, 86594435}});
}

TEST_CASE("Move generation test 63", "[move-gen][case-63]")
{
  doPerftTest("8/1p3K2/2P5/8/1P6/1p1q2B1/2r3kp/3Q4 w - - 0 1",
              6,
              {{33, 1071, 28777, 918056, 24387194, 778714079}});
}

TEST_CASE("Move generation test 64", "[move-gen][case-64]")
{
  doPerftTest("3N4/2P4P/8/1B6/1p5b/qk5P/8/B4K2 w - - 0 1",
              6,
              {{30, 512, 14432, 295362, 8429976, 187649671}});
}

TEST_CASE("Move generation test 65", "[move-gen][case-65]")
{
  doPerftTest("5N2/1K1P4/8/5P1Q/5PR1/1k3P2/1p6/2q4n w - - 0 1",
              6,
              {{32, 901, 27905, 761065, 23539558, 646159707}});
}

TEST_CASE("Move generation test 66", "[move-gen][case-66]")
{
  doPerftTest("6K1/8/6p1/p1k5/1qPb1P2/7P/1Q4P1/2B5 w - - 0 1",
              6,
              {{21, 498, 10839, 253350, 5835307, 137483982}});
}

TEST_CASE("Move generation test 67", "[move-gen][case-67]")
{
  doPerftTest("8/2n5/1r6/6N1/P2RPK2/1Pn5/2R5/4b2k w - - 0 1",
              6,
              {{31, 908, 23865, 639703, 17246461, 444989267}});
}

TEST_CASE("Move generation test 68", "[move-gen][case-68]")
{
  doPerftTest("Q3Rr2/N4k2/3P3P/4p3/K4P2/1p6/1B6/8 w - - 0 1",
              6,
              {{35, 273, 10061, 98086, 3635471, 40851102}});
}

TEST_CASE("Move generation test 69", "[move-gen][case-69]")
{
  doPerftTest("3N4/2kp4/8/3b3p/8/rP2P1p1/1K4BP/8 w - - 0 1",
              6,
              {{20, 503, 9313, 228536, 4193703, 102159203}});
}

TEST_CASE("Move generation test 70", "[move-gen][case-70]")
{
  doPerftTest("3n4/6p1/6kp/3P4/1n6/3PP2b/P2P4/2K5 w - - 0 1",
              6,
              {{8, 191, 1550, 36484, 303076, 7065596}});
}

TEST_CASE("Move generation test 71", "[move-gen][case-71]")
{
  doPerftTest("6R1/4kp1b/8/4KR2/1NP5/2Q1P3/8/1Bq5 w - - 0 1",
              6,
              {{48, 802, 30594, 599340, 22090063, 456984192}});
}

TEST_CASE("Move generation test 72", "[move-gen][case-72]")
{
  doPerftTest("3K4/1N6/2P2R2/P1b5/3Rp1p1/7k/6p1/3B4 w - - 0 1",
              6,
              {{34, 540, 16914, 276474, 8785746, 147924993}});
}

TEST_CASE("Move generation test 73", "[move-gen][case-73]")
{
  doPerftTest("R5Q1/8/P3bK2/8/p5P1/5pn1/2k3p1/R5r1 w - - 0 1",
              6,
              {{35, 862, 28336, 680087, 22907324, 555447940}});
}

TEST_CASE("Move generation test 74", "[move-gen][case-74]")
{
  doPerftTest("8/R7/7K/4P3/pp1b4/1P1NP1k1/4p2p/2R5 w - - 0 1",
              6,
              {{40, 862, 28134, 597112, 18780063, 404083696}});
}

TEST_CASE("Move generation test 75", "[move-gen][case-75]")
{
  doPerftTest("8/k6p/7r/8/Np2P2p/PP6/3p2Q1/K2N4 w - - 0 1",
              6,
              {{28, 366, 10343, 164715, 4672715, 80443482}});
}

TEST_CASE("Move generation test 76", "[move-gen][case-76]")
{
  doPerftTest("8/1P6/2N1PnP1/2pP2K1/1p2P3/8/5P2/4k2r w - - 0 1",
              6,
              {{21, 496, 8873, 203444, 3923293, 88124239}});
}

TEST_CASE("Move generation test 77", "[move-gen][case-77]")
{
  doPerftTest("3R4/K1P5/6k1/2P5/2rppP2/5B2/2r3p1/7n w - - 0 1",
              6,
              {{28, 607, 15203, 348716, 8615435, 209898959}});
}

TEST_CASE("Move generation test 78", "[move-gen][case-78]")
{
  doPerftTest("6R1/4pQ2/1k1N1B2/6P1/2pN4/P2bb3/1K6/8 w - - 0 1",
              6,
              {{43, 979, 37795, 771453, 30100399, 577505759}});
}

TEST_CASE("Move generation test 79", "[move-gen][case-79]")
{
  doPerftTest("4N3/1P6/Q1p1kp1K/8/3P4/3p4/1n2pB2/5q2 w - - 0 1",
              6,
              {{30, 607, 17364, 364065, 10658999, 235577633}});
}

TEST_CASE("Move generation test 80", "[move-gen][case-80]")
{
  doPerftTest("1N2n1q1/1p6/8/6p1/3N4/4p1k1/2R2nP1/Q3K3 w - - 0 1",
              6,
              {{36, 1009, 35033, 1014814, 35105652, 1032881391}});
}
