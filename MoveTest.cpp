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

void doPerftTest(const std::string&      fenstr,
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

TEST_CASE("Move generation test 81", "[move-gen][case-81]")
{
  doPerftTest("5K2/k7/8/8/8/8/8/8 b - - 0 1", 6, {{5, 25, 145, 956, 5806, 35804}});
}

TEST_CASE("Move generation test 82", "[move-gen][case-82]")
{
  doPerftTest("8/8/8/1K2k3/8/8/8/8 w - - 0 1", 6, {{8, 57, 356, 2658, 17816, 135366}});
}

TEST_CASE("Move generation test 83", "[move-gen][case-83]")
{
  doPerftTest("8/8/6k1/8/6K1/8/8/8 w - - 0 1", 6, {{5, 36, 217, 1355, 9242, 58348}});
}

TEST_CASE("Move generation test 84", "[move-gen][case-84]")
{
  doPerftTest("8/1k6/8/8/4K3/8/8/8 w - - 0 1", 6, {{8, 63, 494, 2764, 21408, 137166}});
}

TEST_CASE("Move generation test 85", "[move-gen][case-85]")
{
  doPerftTest("8/8/8/6K1/1k6/8/8/8 w - - 0 1", 6, {{8, 64, 440, 2980, 21496, 153990}});
}

TEST_CASE("Move generation test 86", "[move-gen][case-86]")
{
  doPerftTest("5K2/1k6/8/8/8/8/8/8 w - - 0 1", 6, {{5, 40, 260, 1454, 8882, 56596}});
}

TEST_CASE("Move generation test 87", "[move-gen][case-87]")
{
  doPerftTest("4k3/8/8/8/8/8/5K2/8 w - - 0 1", 6, {{8, 40, 275, 1870, 12862, 86730}});
}

TEST_CASE("Move generation test 88", "[move-gen][case-88]")
{
  doPerftTest("8/8/8/8/8/8/3k3K/8 b - - 0 1", 6, {{8, 40, 257, 1372, 9464, 55002}});
}

TEST_CASE("Move generation test 89", "[move-gen][case-89]")
{
  doPerftTest("8/4k3/8/8/8/8/4K3/8 w - - 0 1", 6, {{8, 64, 440, 2974, 21412, 153072}});
}

TEST_CASE("Move generation test 90", "[move-gen][case-90]")
{
  doPerftTest("8/8/3K4/7k/8/8/8/8 b - - 0 1", 6, {{5, 40, 256, 1947, 12378, 88613}});
}

TEST_CASE("Move generation test 91", "[move-gen][case-91]")
{
  doPerftTest(
    "4kb2/8/8/8/8/4K3/8/8 b - - 0 1", 6, {{11, 81, 1076, 7654, 106424, 695086}});
}

TEST_CASE("Move generation test 92", "[move-gen][case-92]")
{
  doPerftTest("8/3k4/8/p7/8/8/8/7K w - - 0 1", 6, {{3, 27, 162, 1296, 7540, 61535}});
}

TEST_CASE("Move generation test 93", "[move-gen][case-93]")
{
  doPerftTest("8/8/8/3n4/2k5/8/8/2K5 w - - 0 1", 6, {{5, 68, 338, 4728, 22517, 302529}});
}

TEST_CASE("Move generation test 94", "[move-gen][case-94]")
{
  doPerftTest("4k3/8/P7/8/8/8/5K2/8 b - - 0 1", 6, {{5, 45, 306, 2550, 16840, 150516}});
}

TEST_CASE("Move generation test 95", "[move-gen][case-95]")
{
  doPerftTest("8/8/5k2/8/3b4/K7/8/8 w - - 0 1", 6, {{4, 72, 384, 6785, 37356, 627841}});
}

TEST_CASE("Move generation test 96", "[move-gen][case-96]")
{
  doPerftTest("8/8/5k2/8/2K3P1/8/8/8 b - - 0 1", 6, {{7, 59, 402, 3480, 21370, 175455}});
}

TEST_CASE("Move generation test 97", "[move-gen][case-97]")
{
  doPerftTest("8/5k2/8/8/6K1/8/p7/8 w - - 0 1", 6, {{8, 90, 563, 7329, 45710, 734870}});
}

TEST_CASE("Move generation test 98", "[move-gen][case-98]")
{
  doPerftTest("8/8/1Q6/8/7k/K7/8/8 b - - 0 1", 6, {{5, 140, 692, 18671, 87251, 2367227}});
}

TEST_CASE("Move generation test 99", "[move-gen][case-99]")
{
  doPerftTest("8/4P3/8/k7/8/8/2K5/8 w - - 0 1", 6, {{12, 53, 675, 3830, 59839, 310958}});
}

TEST_CASE("Move generation test 100", "[move-gen][case-100]")
{
  doPerftTest("8/8/1q6/5k1K/8/8/8/8 w - - 0 1", 6, {{1, 29, 80, 2224, 7547, 213151}});
}

TEST_CASE("Move generation test 101", "[move-gen][case-101]")
{
  doPerftTest("8/8/8/5N2/1k1K4/7p/8/8 b - - 0 1", 6, {{6, 83, 558, 7452, 58469, 743015}});
}

TEST_CASE("Move generation test 102", "[move-gen][case-102]")
{
  doPerftTest("5K2/8/7k/6p1/4p3/8/8/8 b - - 0 1", 6, {{5, 19, 110, 603, 3919, 22356}});
}

TEST_CASE("Move generation test 103", "[move-gen][case-103]")
{
  doPerftTest(
    "8/1K4k1/8/q7/8/6p1/8/8 w - - 0 1", 6, {{3, 90, 305, 8868, 39799, 1132425}});
}

TEST_CASE("Move generation test 104", "[move-gen][case-104]")
{
  doPerftTest(
    "8/4k3/8/3K4/p7/8/8/1Q6 b - - 0 1", 6, {{7, 195, 1060, 28546, 153124, 4136248}});
}

TEST_CASE("Move generation test 105", "[move-gen][case-105]")
{
  doPerftTest(
    "8/8/1r6/7k/2K5/8/1B6/8 b - - 0 1", 6, {{18, 237, 4149, 54176, 950365, 12738140}});
}

TEST_CASE("Move generation test 106", "[move-gen][case-106]")
{
  doPerftTest("2K5/8/7b/8/8/8/7p/1k6 w - - 0 1", 6, {{5, 80, 510, 9129, 51694, 1032724}});
}

TEST_CASE("Move generation test 107", "[move-gen][case-107]")
{
  doPerftTest(
    "1k6/8/2K5/7P/5r2/8/8/8 b - - 0 1", 6, {{17, 105, 1824, 11600, 201206, 1273144}});
}

TEST_CASE("Move generation test 108", "[move-gen][case-108]")
{
  doPerftTest("8/2K5/q5p1/4k3/8/8/8/8 w - - 0 1", 6, {{3, 82, 257, 7053, 26955, 749445}});
}

TEST_CASE("Move generation test 109", "[move-gen][case-109]")
{
  doPerftTest(
    "8/8/8/1k2K3/8/1p6/4n3/8 b - - 0 1", 6, {{15, 89, 1175, 7735, 103239, 687365}});
}

TEST_CASE("Move generation test 110", "[move-gen][case-110]")
{
  doPerftTest("k7/4n3/8/8/2P5/8/7K/8 w - - 0 1", 6, {{6, 54, 354, 3508, 24016, 243351}});
}

TEST_CASE("Move generation test 111", "[move-gen][case-111]")
{
  doPerftTest(
    "2N2R2/8/4P3/8/k7/6K1/8/8 b - - 0 1", 6, {{5, 120, 730, 17535, 100215, 2422527}});
}

TEST_CASE("Move generation test 112", "[move-gen][case-112]")
{
  doPerftTest(
    "8/1K6/3B4/8/8/2P5/8/5kn1 b - - 0 1", 6, {{7, 140, 1320, 22631, 218527, 3448686}});
}

TEST_CASE("Move generation test 113", "[move-gen][case-113]")
{
  doPerftTest(
    "1R6/7P/4k3/8/3K4/8/8/6B1 w - - 0 1", 6, {{27, 152, 4358, 22861, 682204, 3474284}});
}

TEST_CASE("Move generation test 114", "[move-gen][case-114]")
{
  doPerftTest("5N2/8/4Q3/3n4/8/8/2k5/4K3 w - - 0 1",
              6,
              {{27, 298, 8361, 82864, 2318083, 21945538}});
}

TEST_CASE("Move generation test 115", "[move-gen][case-115]")
{
  doPerftTest(
    "1kr5/3P4/8/p7/7K/8/8/8 w - - 0 1", 6, {{13, 135, 1473, 19047, 228079, 3277615}});
}

TEST_CASE("Move generation test 116", "[move-gen][case-116]")
{
  doPerftTest(
    "4K3/8/8/1P6/3k4/4p3/8/q7 w - - 0 1", 6, {{6, 144, 947, 24248, 147386, 3800080}});
}

TEST_CASE("Move generation test 117", "[move-gen][case-117]")
{
  doPerftTest(
    "8/8/3n4/k6K/2p5/8/8/1n6 w - - 0 1", 6, {{5, 80, 481, 7652, 45664, 729085}});
}

TEST_CASE("Move generation test 118", "[move-gen][case-118]")
{
  doPerftTest(
    "8/5kP1/8/8/1K6/6P1/8/7r b - - 0 1", 6, {{21, 248, 4297, 47922, 785322, 9393781}});
}

TEST_CASE("Move generation test 119", "[move-gen][case-119]")
{
  doPerftTest(
    "8/1k6/8/7K/Q7/8/4p3/3r4 w - - 0 1", 6, {{26, 418, 9811, 161432, 3795689, 65988645}});
}

TEST_CASE("Move generation test 120", "[move-gen][case-120]")
{
  doPerftTest(
    "3r4/8/P7/8/8/K7/2p5/4k3 b - - 0 1", 6, {{23, 125, 2891, 18042, 423927, 2775921}});
}

TEST_CASE("Move generation test 121", "[move-gen][case-121]")
{
  doPerftTest(
    "3k4/1b6/8/7P/4P3/P7/8/5K2 w - - 0 1", 6, {{8, 91, 768, 9816, 83495, 1121245}});
}

TEST_CASE("Move generation test 122", "[move-gen][case-122]")
{
  doPerftTest(
    "2B5/8/8/1k6/3P4/3P1P2/8/4K3 b - - 0 1", 6, {{5, 70, 360, 5181, 26997, 391521}});
}

TEST_CASE("Move generation test 123", "[move-gen][case-123]")
{
  doPerftTest(
    "8/6K1/1P6/4k3/8/6N1/3R4/2Q5 b - - 0 1", 6, {{2, 88, 316, 13835, 45956, 1969690}});
}

TEST_CASE("Move generation test 124", "[move-gen][case-124]")
{
  doPerftTest(
    "k7/8/8/2P5/4p3/6R1/1K5n/8 b - - 0 1", 6, {{7, 148, 1254, 24045, 214255, 3894813}});
}

TEST_CASE("Move generation test 125", "[move-gen][case-125]")
{
  doPerftTest(
    "5k2/P7/6K1/8/2pq4/8/1p6/8 w - - 0 1", 6, {{9, 221, 1408, 37862, 287157, 7939140}});
}

TEST_CASE("Move generation test 126", "[move-gen][case-126]")
{
  doPerftTest(
    "6k1/6N1/6P1/4p3/6K1/p7/8/8 b - - 0 1", 6, {{5, 51, 274, 2583, 18049, 183076}});
}

TEST_CASE("Move generation test 127", "[move-gen][case-127]")
{
  doPerftTest("8/P7/8/1k6/4B3/2K2b2/6p1/8 w - - 0 1",
              6,
              {{21, 302, 5644, 93244, 1709159, 30714304}});
}

TEST_CASE("Move generation test 128", "[move-gen][case-128]")
{
  doPerftTest("8/8/8/8/K2P1bP1/6Q1/8/3k4 w - - 0 1",
              6,
              {{21, 221, 4946, 47073, 1095418, 10824557}});
}

TEST_CASE("Move generation test 129", "[move-gen][case-129]")
{
  doPerftTest(
    "1q6/4p3/1b6/4k3/8/3K4/7P/8 w - - 0 1", 6, {{7, 196, 1363, 43043, 268459, 8784906}});
}

TEST_CASE("Move generation test 130", "[move-gen][case-130]")
{
  doPerftTest(
    "8/5RP1/2k3P1/P7/8/3K4/8/8 w - - 0 1", 6, {{25, 111, 2716, 14734, 373473, 1794856}});
}

TEST_CASE("Move generation test 131", "[move-gen][case-131]")
{
  doPerftTest(
    "K4N1k/2P5/8/7P/6R1/4p3/8/8 b - - 0 1", 6, {{1, 26, 118, 3068, 33035, 794419}});
}

TEST_CASE("Move generation test 132", "[move-gen][case-132]")
{
  doPerftTest("1k4K1/1P3P2/5P1r/5Q2/8/8/8/8 w - - 0 1",
              6,
              {{28, 266, 6273, 74366, 1884770, 23580888}});
}

TEST_CASE("Move generation test 133", "[move-gen][case-133]")
{
  doPerftTest("7B/8/8/2k5/2p5/P7/3br3/1K6 b - - 0 1",
              6,
              {{25, 260, 6269, 69067, 1652897, 18708551}});
}

TEST_CASE("Move generation test 134", "[move-gen][case-134]")
{
  doPerftTest("b7/8/1B5K/3k4/3r2pP/8/8/8 b - - 0 1",
              6,
              {{17, 228, 4604, 62072, 1340158, 17985036}});
}

TEST_CASE("Move generation test 135", "[move-gen][case-135]")
{
  doPerftTest(
    "6r1/2n5/p7/8/4K3/8/4k3/1r2b3 w - - 0 1", 6, {{4, 172, 710, 29835, 123079, 5096099}});
}

TEST_CASE("Move generation test 136", "[move-gen][case-136]")
{
  doPerftTest("k7/2K5/4p3/2Q5/8/5n1P/6r1/8 w - - 0 1",
              6,
              {{30, 567, 14272, 238211, 5737064, 94085302}});
}

TEST_CASE("Move generation test 137", "[move-gen][case-137]")
{
  doPerftTest(
    "7K/8/8/6kp/5b2/6B1/pP6/8 b - - 0 1", 6, {{18, 180, 3037, 33129, 593626, 6779896}});
}

TEST_CASE("Move generation test 138", "[move-gen][case-138]")
{
  doPerftTest("8/Q3K1k1/R6b/8/p3n3/8/8/8 w - - 0 1",
              6,
              {{24, 317, 9153, 129758, 3867489, 56793189}});
}

TEST_CASE("Move generation test 139", "[move-gen][case-139]")
{
  doPerftTest(
    "8/6p1/P5p1/K7/3k1P2/8/3p4/8 w - - 0 1", 6, {{6, 68, 504, 6635, 54417, 852255}});
}

TEST_CASE("Move generation test 140", "[move-gen][case-140]")
{
  doPerftTest(
    "8/8/8/5nNk/7P/2p5/5K1P/8 b - - 0 1", 6, {{13, 180, 1948, 23709, 262598, 3088111}});
}

TEST_CASE("Move generation test 141", "[move-gen][case-141]")
{
  doPerftTest("k1K5/7N/P7/3r4/1R2p3/8/1p6/8 b - - 0 1",
              6,
              {{20, 288, 5205, 83585, 1583575, 26576943}});
}

TEST_CASE("Move generation test 142", "[move-gen][case-142]")
{
  doPerftTest("8/5p2/4p3/1K4N1/3N4/R7/3kP3/8 b - - 0 1",
              6,
              {{6, 216, 1318, 43510, 267863, 8392064}});
}

TEST_CASE("Move generation test 143", "[move-gen][case-143]")
{
  doPerftTest("8/8/2k5/2p5/K1p5/8/b2P1P1R/8 b - - 0 1",
              6,
              {{9, 119, 1253, 20115, 226927, 3868912}});
}

TEST_CASE("Move generation test 144", "[move-gen][case-144]")
{
  doPerftTest(
    "8/1p6/8/1P2pp2/8/2P5/K2k4/2R5 b - - 0 1", 6, {{7, 94, 694, 10278, 74657, 1192931}});
}

TEST_CASE("Move generation test 145", "[move-gen][case-145]")
{
  doPerftTest(
    "8/6N1/p7/7P/1KR5/8/R6Q/3k4 w - - 0 1", 6, {{49, 80, 2592, 5498, 218562, 448120}});
}

TEST_CASE("Move generation test 146", "[move-gen][case-146]")
{
  doPerftTest("N7/8/1K6/PQp5/2R5/6r1/k7/8 w - - 0 1",
              6,
              {{29, 396, 11919, 158441, 5114804, 68480293}});
}

TEST_CASE("Move generation test 147", "[move-gen][case-147]")
{
  doPerftTest(
    "8/1r6/n5P1/3p4/3K2p1/6b1/3k4/8 w - - 0 1", 6, {{2, 66, 349, 10953, 71170, 2112538}});
}

TEST_CASE("Move generation test 148", "[move-gen][case-148]")
{
  doPerftTest("8/8/3b2P1/1K6/1n4bk/8/4r3/1B6 b - - 0 1",
              6,
              {{40, 392, 15179, 168580, 6352186, 77058459}});
}

TEST_CASE("Move generation test 149", "[move-gen][case-149]")
{
  doPerftTest("8/8/3b2pK/8/1k2B1b1/2n3p1/8/8 b - - 0 1",
              6,
              {{33, 479, 14614, 186251, 5453893, 68867868}});
}

TEST_CASE("Move generation test 150", "[move-gen][case-150]")
{
  doPerftTest("6k1/1p1N4/3p4/8/1PK5/5N2/8/5n2 w - - 0 1",
              6,
              {{21, 222, 3503, 38772, 634209, 7211558}});
}

TEST_CASE("Move generation test 151", "[move-gen][case-151]")
{
  doPerftTest("8/P3k3/3q4/P7/2P4p/1Kp5/8/6r1 b - - 0 1",
              6,
              {{46, 439, 18332, 188770, 6988775, 83029410}});
}

TEST_CASE("Move generation test 152", "[move-gen][case-152]")
{
  doPerftTest("8/k5p1/4qP1N/P7/8/4P1K1/8/B7 w - - 0 1",
              6,
              {{18, 484, 7652, 190887, 3100780, 74577687}});
}

TEST_CASE("Move generation test 153", "[move-gen][case-153]")
{
  doPerftTest("8/4r3/n2b4/2p3P1/7K/2k2p2/8/6n1 w - - 0 1",
              6,
              {{3, 105, 442, 15215, 69457, 2342991}});
}

TEST_CASE("Move generation test 154", "[move-gen][case-154]")
{
  doPerftTest("1k4r1/4p2R/5p2/1P3r2/8/3q4/1K6/8 w - - 0 1",
              6,
              {{14, 682, 8121, 370382, 4576457, 199863572}});
}

TEST_CASE("Move generation test 155", "[move-gen][case-155]")
{
  doPerftTest("8/1p1P2q1/8/8/K2kPR2/8/n7/7N w - - 0 1",
              6,
              {{20, 467, 9015, 223803, 4390548, 108030120}});
}

TEST_CASE("Move generation test 156", "[move-gen][case-156]")
{
  doPerftTest("8/6p1/4Rrp1/7k/2pN4/7p/3K4/8 b - - 0 1",
              6,
              {{15, 340, 5896, 127883, 2207685, 46449010}});
}

TEST_CASE("Move generation test 157", "[move-gen][case-157]")
{
  doPerftTest("5n2/8/P1P3Q1/8/1k6/r5P1/6R1/5K2 b - - 0 1",
              6,
              {{22, 624, 10981, 330080, 5685543, 175308898}});
}

TEST_CASE("Move generation test 158", "[move-gen][case-158]")
{
  doPerftTest("8/3R4/8/4p2k/4P1p1/1p1P4/8/3K1b2 w - - 0 1",
              6,
              {{15, 143, 2074, 21622, 321221, 3636668}});
}

TEST_CASE("Move generation test 159", "[move-gen][case-159]")
{
  doPerftTest("8/1K5p/2p1P1R1/8/5p2/5k2/3B4/1B6 b - - 0 1",
              6,
              {{6, 172, 1090, 31674, 219721, 6431591}});
}

TEST_CASE("Move generation test 160", "[move-gen][case-160]")
{
  doPerftTest("r7/3B2p1/7P/8/5n2/1P6/3k4/5QK1 b - - 0 1",
              6,
              {{28, 798, 15903, 466008, 9055034, 270779469}});
}

TEST_CASE("Move generation test 161", "[move-gen][case-161]")
{
  doPerftTest("2R2b2/N1K5/1N2k3/8/1n6/6Pq/4P3/8 b - - 0 1",
              6,
              {{28, 435, 12533, 229747, 6713317, 134020121}});
}

TEST_CASE("Move generation test 162", "[move-gen][case-162]")
{
  doPerftTest("K7/8/P3Q3/8/P7/p6N/3PPp2/4k3 w - - 0 1",
              6,
              {{34, 280, 9344, 84669, 2577810, 32312315}});
}

TEST_CASE("Move generation test 163", "[move-gen][case-163]")
{
  doPerftTest("4k3/7n/8/1p6/1p4P1/R3p3/2K4p/4b3 w - - 0 1",
              6,
              {{18, 313, 4845, 91591, 1378376, 28051505}});
}

TEST_CASE("Move generation test 164", "[move-gen][case-164]")
{
  doPerftTest("3q4/1K6/2pP4/5R2/4r2p/3B2bk/8/8 b - - 0 1",
              6,
              {{38, 781, 27233, 555719, 18948567, 387592801}});
}

TEST_CASE("Move generation test 165", "[move-gen][case-165]")
{
  doPerftTest("3k2K1/7n/8/p1n5/4rP2/p3P3/7N/8 b - - 0 1",
              6,
              {{27, 210, 5517, 48478, 1253504, 11851951}});
}

TEST_CASE("Move generation test 166", "[move-gen][case-166]")
{
  doPerftTest("8/8/2R1N3/5pP1/6n1/7r/3nKpk1/8 b - - 0 1",
              6,
              {{34, 625, 18886, 369291, 10818186, 208259181}});
}

TEST_CASE("Move generation test 167", "[move-gen][case-167]")
{
  doPerftTest("3B4/2p3K1/8/N7/8/8/1k1PqP2/2n2r2 w - - 0 1",
              6,
              {{21, 621, 10437, 328394, 5246036, 170871661}});
}

TEST_CASE("Move generation test 168", "[move-gen][case-168]")
{
  doPerftTest("8/pK6/1pp1P3/6R1/2kB2P1/8/P7/8 b - - 0 1",
              6,
              {{7, 211, 1526, 41913, 314249, 8303341}});
}

TEST_CASE("Move generation test 169", "[move-gen][case-169]")
{
  doPerftTest("8/7P/2P1P3/pK5R/3NP3/3k2p1/8/8 w - - 0 1",
              6,
              {{27, 183, 5021, 35589, 1006490, 7981208}});
}

TEST_CASE("Move generation test 170", "[move-gen][case-170]")
{
  doPerftTest("N7/kB3K1b/r2pP3/8/2P5/8/8/B7 b - - 0 1",
              6,
              {{17, 352, 6548, 139039, 2703159, 58391313}});
}

TEST_CASE("Move generation test 171", "[move-gen][case-171]")
{
  doPerftTest("8/8/1P1k4/P4K2/2P2p1B/7N/1p4Q1/q7 b - - 0 1",
              6,
              {{18, 590, 10621, 305655, 6035728, 166685459}});
}

TEST_CASE("Move generation test 172", "[move-gen][case-172]")
{
  doPerftTest("7k/1p6/1R5p/4q3/6p1/6b1/1p4P1/4n2K b - - 0 1",
              6,
              {{37, 461, 17187, 214037, 7915747, 98737869}});
}

TEST_CASE("Move generation test 173", "[move-gen][case-173]")
{
  doPerftTest("8/1b6/2k4p/2BpP3/3PN3/8/1R4R1/6K1 w - - 0 1",
              6,
              {{41, 270, 10479, 89315, 3367812, 30760656}});
}

TEST_CASE("Move generation test 174", "[move-gen][case-174]")
{
  doPerftTest("6n1/2qp1B2/6P1/1n3K1P/8/2b4p/8/5k2 w - - 0 1",
              6,
              {{12, 451, 4912, 188051, 2091895, 79780192}});
}

TEST_CASE("Move generation test 175", "[move-gen][case-175]")
{
  doPerftTest("7K/Pk1q4/8/3n4/4PP2/1p2n2R/4r3/8 w - - 0 1",
              6,
              {{17, 669, 10345, 405366, 6496933, 253515421}});
}

TEST_CASE("Move generation test 176", "[move-gen][case-176]")
{
  doPerftTest("8/5p2/2k3P1/7P/1R2K3/8/5pP1/3qNn2 w - - 0 1",
              6,
              {{21, 606, 9066, 261701, 4317654, 125715631}});
}

TEST_CASE("Move generation test 177", "[move-gen][case-177]")
{
  doPerftTest("2N5/P5B1/8/4Np1p/8/4r3/1kPb4/5K2 b - - 0 1",
              6,
              {{25, 547, 10837, 260998, 5501388, 139728066}});
}

TEST_CASE("Move generation test 178", "[move-gen][case-178]")
{
  doPerftTest("4k3/N3r3/8/7p/7P/3B2P1/p1K5/rR6 b - - 0 1",
              6,
              {{22, 547, 11252, 250867, 5382748, 116022801}});
}

TEST_CASE("Move generation test 179", "[move-gen][case-179]")
{
  doPerftTest("1R4q1/1P2b3/P3P3/8/K1Q5/3R4/1p6/1k6 b - - 0 1",
              6,
              {{26, 1026, 24171, 937364, 22674103, 868503801}});
}

TEST_CASE("Move generation test 180", "[move-gen][case-180]")
{
  doPerftTest("4N2b/8/4K2P/5R1P/3r1P2/8/1n4Q1/k7 w - - 0 1",
              6,
              {{38, 669, 21716, 397228, 12753352, 245560779}});
}

TEST_CASE("Move generation test 181", "[move-gen][case-181]")
{
  doPerftTest("8/1P4PP/6B1/2K2p1b/4p1pp/6k1/1n6/8 b - - 0 1",
              6,
              {{14, 287, 4103, 93735, 1404334, 34504738}});
}

TEST_CASE("Move generation test 182", "[move-gen][case-182]")
{
  doPerftTest("3N4/K7/2r5/3R2b1/P4p1P/5P2/4P1k1/7b w - - 0 1",
              6,
              {{24, 551, 12122, 265320, 5710281, 124419522}});
}

TEST_CASE("Move generation test 183", "[move-gen][case-183]")
{
  doPerftTest("7n/1N6/8/2P3k1/2R5/2Ppp3/4p1n1/K2B4 b - - 0 1",
              6,
              {{19, 297, 4973, 93086, 1584577, 32021206}});
}

TEST_CASE("Move generation test 184", "[move-gen][case-184]")
{
  doPerftTest("8/N2P1P2/5R2/8/2k3PK/8/PBb2p2/1B6 b - - 0 1",
              6,
              {{17, 638, 9265, 361964, 5463149, 215086478}});
}

TEST_CASE("Move generation test 185", "[move-gen][case-185]")
{
  doPerftTest("r7/8/1R5P/2b5/3P4/4K2P/1rR1P1p1/7k b - - 0 1",
              6,
              {{34, 767, 24080, 552585, 17256193, 406001942}});
}

TEST_CASE("Move generation test 186", "[move-gen][case-186]")
{
  doPerftTest("8/4r1q1/1n1bP3/R7/7k/3pP3/3K4/3BN3 w - - 0 1",
              6,
              {{28, 1021, 24280, 905361, 21229926, 798674109}});
}

TEST_CASE("Move generation test 187", "[move-gen][case-187]")
{
  doPerftTest("8/1Pn2q2/1k6/1r4p1/2p1P3/8/R2KP2n/8 b - - 0 1",
              6,
              {{39, 677, 23011, 392453, 13207868, 224427618}});
}

TEST_CASE("Move generation test 188", "[move-gen][case-188]")
{
  doPerftTest("8/2bnP3/8/2Nb1K1p/1p6/8/1r1p2p1/4k3 w - - 0 1",
              6,
              {{14, 488, 6430, 243875, 3423673, 129070780}});
}

TEST_CASE("Move generation test 189", "[move-gen][case-189]")
{
  doPerftTest("8/3P4/3K4/2B1P1bP/4pp2/2P5/7k/1r3q2 b - - 0 1",
              6,
              {{36, 580, 20052, 345754, 11954355, 215325036}});
}

TEST_CASE("Move generation test 190", "[move-gen][case-190]")
{
  doPerftTest("8/4k1PK/8/1B6/p5P1/P4Pbp/6n1/7R b - - 0 1",
              6,
              {{19, 514, 8672, 226672, 3770122, 98163081}});
}

TEST_CASE("Move generation test 191", "[move-gen][case-191]")
{
  doPerftTest("7B/4Q3/1NK5/4p1p1/3R4/2P3P1/3rR3/1Bk5 b - - 0 1",
              6,
              {{13, 796, 9235, 500090, 5929762, 299806423}});
}

TEST_CASE("Move generation test 192", "[move-gen][case-192]")
{
  doPerftTest("8/k4Pp1/2pR2q1/2P1nr2/6Q1/1K6/1Pp5/8 w - - 0 1",
              6,
              {{39, 1166, 37980, 1240851, 39155096, 1333120049}});
}

TEST_CASE("Move generation test 193", "[move-gen][case-193]")
{
  doPerftTest("8/7Q/b5p1/n7/1KP3PP/2P4p/3BkP2/8 w - - 0 1",
              6,
              {{27, 407, 10544, 158724, 4241967, 66457707}});
}

TEST_CASE("Move generation test 194", "[move-gen][case-194]")
{
  doPerftTest("3n4/2k1P1qb/4B3/1Kb3P1/5P2/P2P4/7p/8 w - - 0 1",
              6,
              {{28, 1002, 23166, 828028, 17290144, 623619726}});
}

TEST_CASE("Move generation test 195", "[move-gen][case-195]")
{
  doPerftTest("8/1P3Bpp/8/1p3K2/7b/1b2BP1k/1N1R4/8 w - - 0 1",
              6,
              {{41, 831, 29284, 568349, 19827210, 379273337}});
}

TEST_CASE("Move generation test 196", "[move-gen][case-196]")
{
  doPerftTest("8/2qp3k/2Q1P3/p7/6n1/1RN1p2P/B7/6K1 b - - 0 1",
              6,
              {{28, 975, 25704, 907670, 23776769, 839504330}});
}

TEST_CASE("Move generation test 197", "[move-gen][case-197]")
{
  doPerftTest("8/p1p4B/5K2/3r1PP1/3p4/N6r/5P2/5kb1 b - - 0 1",
              6,
              {{32, 405, 12478, 163253, 4851257, 66013344}});
}

TEST_CASE("Move generation test 198", "[move-gen][case-198]")
{
  doPerftTest("1B1rb3/3P3P/3P4/K7/7b/3k1PqB/p7/8 b - - 0 1",
              6,
              {{36, 642, 22950, 448215, 16489045, 344448657}});
}

TEST_CASE("Move generation test 199", "[move-gen][case-199]")
{
  doPerftTest("8/4P3/Qpp1p3/1P1b3P/4k3/5R2/2P5/4b1K1 b - - 0 1",
              6,
              {{16, 521, 8544, 272350, 4584911, 146110481}});
}

TEST_CASE("Move generation test 200", "[move-gen][case-200]")
{
  doPerftTest("2K1N3/BP6/p4Pp1/8/6r1/3pPp2/1k6/5b2 w - - 0 1",
              6,
              {{17, 380, 6753, 146723, 2759423, 58614505}});
}

TEST_CASE("Move generation test 201", "[move-gen][case-201]")
{
  doPerftTest("8/4p1Pr/2p1kN2/6p1/QK6/6n1/1P4p1/2Bb4 w - - 0 1",
              6,
              {{34, 943, 31103, 817591, 27120507, 703662169}});
}

TEST_CASE("Move generation test 202", "[move-gen][case-202]")
{
  doPerftTest("5b2/8/P4P2/1P2k2p/1p4n1/1p1p2r1/2R5/1KR5 w - - 0 1",
              6,
              {{23, 548, 13281, 322870, 7705882, 190935008}});
}

TEST_CASE("Move generation test 203", "[move-gen][case-203]")
{
  doPerftTest("1b6/rb1qP3/8/nPP5/BQ2p3/8/1R6/k5K1 w - - 0 1",
              6,
              {{31, 931, 28378, 823870, 26350744, 794359748}});
}

TEST_CASE("Move generation test 204", "[move-gen][case-204]")
{
  doPerftTest("8/2p1P2b/1p5b/4Q3/1kq1Pp2/1P3PR1/8/6K1 b - - 0 1",
              6,
              {{30, 875, 22558, 668331, 17593731, 525738856}});
}

TEST_CASE("Move generation test 205", "[move-gen][case-205]")
{
  doPerftTest("8/7P/2p5/1np4B/5KpB/n1k4P/2p3p1/7q w - - 0 1",
              6,
              {{23, 634, 12288, 354264, 6863898, 205777464}});
}

TEST_CASE("Move generation test 206", "[move-gen][case-206]")
{
  doPerftTest("R7/8/1PPk4/6Pp/p1n2R1r/1B1P4/P4K2/8 b - - 0 1",
              6,
              {{20, 717, 11755, 389557, 6490965, 205074402}});
}

TEST_CASE("Move generation test 207", "[move-gen][case-207]")
{
  doPerftTest("3b4/2p1R3/7P/k2P1r2/7Q/p2P1q2/6n1/6KN w - - 0 1",
              6,
              {{33, 1006, 27799, 934058, 24704613, 860073395}});
}

TEST_CASE("Move generation test 208", "[move-gen][case-208]")
{
  doPerftTest("3K4/1p3k2/P7/2B1b1n1/1P6/PN3P2/2B5/5rQ1 w - - 0 1",
              6,
              {{37, 1159, 40557, 1113172, 40428521, 1030939158}});
}

TEST_CASE("Move generation test 209", "[move-gen][case-209]")
{
  doPerftTest("1R6/K5p1/3P1P2/2P3k1/B6n/5P1b/4P2P/2q5 b - - 0 1",
              6,
              {{35, 1059, 34803, 939032, 29787600, 764409454}});
}

TEST_CASE("Move generation test 210", "[move-gen][case-210]")
{
  doPerftTest("2b4q/2k5/6PP/p4P2/1Q6/2p2BrP/3b3K/8 b - - 0 1",
              6,
              {{33, 1135, 31853, 969942, 29539732, 845491856}});
}

TEST_CASE("Move generation test 211", "[move-gen][case-211]")
{
  doPerftTest("nB1r1k2/p5bp/2PpP3/7Q/1P6/6K1/4qp2/8 b - - 0 1",
              6,
              {{42, 1069, 36880, 843454, 29492698, 670822855}});
}

TEST_CASE("Move generation test 212", "[move-gen][case-212]")
{
  doPerftTest("8/1k5p/1P5B/3NP1qN/1P4Q1/3ppP2/2P5/1K6 b - - 0 1",
              6,
              {{20, 736, 14741, 506100, 10280963, 340167590}});
}

TEST_CASE("Move generation test 213", "[move-gen][case-213]")
{
  doPerftTest("3k4/pQ5q/p2P4/K6P/B4b2/4Pn2/P3p3/5r2 b - - 0 1",
              6,
              {{43, 1314, 44153, 1202984, 41541342, 1093860666}});
}

TEST_CASE("Move generation test 214", "[move-gen][case-214]")
{
  doPerftTest("8/1pr1B1RK/7P/3prpP1/8/4P1k1/Q2P4/7q b - - 0 1",
              6,
              {{39, 1122, 42295, 1261683, 46287532, 1396635218}});
}

TEST_CASE("Move generation test 215", "[move-gen][case-215]")
{
  doPerftTest("6nR/1pN1P1k1/8/5p2/P1pK3P/pp2P3/8/1Q6 w - - 0 1",
              6,
              {{36, 401, 14616, 166005, 5939281, 71002669}});
}

TEST_CASE("Move generation test 216", "[move-gen][case-216]")
{
  doPerftTest("1rk2nq1/P5BN/8/1R2R3/p3K1p1/3PP3/1p6/8 w - - 0 1",
              6,
              {{36, 762, 23814, 565236, 17157444, 441320782}});
}

TEST_CASE("Move generation test 217", "[move-gen][case-217]")
{
  doPerftTest("8/5P2/1p2R1b1/p2PK1Pp/1NP5/4nP2/pk6/8 b - - 0 1",
              6,
              {{29, 634, 16684, 383056, 9594114, 226789726}});
}

TEST_CASE("Move generation test 218", "[move-gen][case-218]")
{
  doPerftTest("8/1P1K2p1/5B2/7p/5r2/2p4P/1PN1Qpn1/2k2b2 b - - 0 1",
              6,
              {{21, 904, 19067, 751910, 17281231, 650935265}});
}

TEST_CASE("Move generation test 219", "[move-gen][case-219]")
{
  doPerftTest("8/b1P4r/4PB2/1PP5/P1k2P2/8/3pP3/1K2R2Q b - - 0 1",
              6,
              {{26, 969, 22140, 807626, 17874090, 649387923}});
}

TEST_CASE("Move generation test 220", "[move-gen][case-220]")
{
  doPerftTest("8/6kp/N1K3p1/4P1P1/2pq1pPB/8/2Rr3p/8 w - - 0 1",
              6,
              {{17, 575, 8782, 310028, 5330966, 189003897}});
}

TEST_CASE("Move generation test 221", "[move-gen][case-221]")
{
  doPerftTest("5N2/p1B5/Q1p5/3PK1p1/nr3P1p/7k/3p1qp1/8 b - - 0 1",
              6,
              {{41, 1031, 40842, 1008660, 40806793, 1014397241}});
}

TEST_CASE("Move generation test 222", "[move-gen][case-222]")
{
  doPerftTest("5k1n/8/1K6/4B1pp/5r1P/2pr2p1/1P1p2PP/2R5 w - - 0 1",
              6,
              {{32, 1108, 29648, 972794, 23891988, 776952627}});
}

TEST_CASE("Move generation test 223", "[move-gen][case-223]")
{
  doPerftTest("6N1/P7/4rpP1/4kPb1/3RP3/5np1/P1b2N2/5K2 b - - 0 1",
              6,
              {{28, 660, 16600, 393164, 9962357, 240206225}});
}

TEST_CASE("Move generation test 224", "[move-gen][case-224]")
{
  doPerftTest("Rq6/1P4k1/4b1p1/3PNpB1/2Q3p1/2R5/1P4K1/4b3 b - - 0 1",
              6,
              {{28, 1639, 42282, 2396450, 61791718, 3391618965}});
}

TEST_CASE("Move generation test 225", "[move-gen][case-225]")
{
  doPerftTest("BK6/2p5/1P2p1Pp/1PpPN2k/b7/n7/1B5p/8 w - - 0 1",
              6,
              {{23, 419, 10199, 202678, 5035084, 107532251}});
}

TEST_CASE("Move generation test 226", "[move-gen][case-226]")
{
  doPerftTest("3K4/N1p1P3/PppB4/4p2b/8/pp1n4/1b2P3/2k5 b - - 0 1",
              6,
              {{23, 395, 9476, 174483, 4289989, 85496737}});
}

TEST_CASE("Move generation test 227", "[move-gen][case-227]")
{
  doPerftTest("2q5/R1R2pPn/4P2P/b5p1/K4Pp1/5k2/2B5/5b2 b - - 0 1",
              6,
              {{38, 1101, 38289, 1117818, 38655152, 1140971275}});
}

TEST_CASE("Move generation test 228", "[move-gen][case-228]")
{
  doPerftTest("7B/3Pkp1b/3n1N2/8/1p6/1p1pK1Pn/P2p4/3B4 b - - 0 1",
              6,
              {{20, 464, 8626, 205031, 4052248, 98128121}});
}

TEST_CASE("Move generation test 229", "[move-gen][case-229]")
{
  doPerftTest("8/PP3QP1/PR1p1p2/P7/3r2k1/bp4p1/1p6/7K b - - 0 1",
              6,
              {{24, 639, 12875, 370123, 7540907, 227890001}});
}

TEST_CASE("Move generation test 230", "[move-gen][case-230]")
{
  doPerftTest("8/N7/K5NP/P2P2q1/6Q1/2p5/P1Bkp2p/2b2R2 b - - 0 1",
              6,
              {{29, 1298, 36177, 1509807, 43471818, 1736553271}});
}

TEST_CASE("Move generation test 231", "[move-gen][case-231]")
{
  doPerftTest("B7/2Pp3r/1bp1k3/pP6/p3Kn1p/qr1P2P1/8/8 w - - 0 1",
              6,
              {{13, 568, 7069, 293685, 3750553, 151790749}});
}

TEST_CASE("Move generation test 232", "[move-gen][case-232]")
{
  doPerftTest("5q2/p4r1P/1P3N2/4pp2/3B1k2/rp3P2/4P3/4b1Kb b - - 0 1",
              6,
              {{42, 940, 32399, 723153, 25585901, 573816601}});
}

TEST_CASE("Move generation test 233", "[move-gen][case-233]")
{
  doPerftTest("2k4b/2P5/7P/1KP5/1P1pN3/1PB1nB2/pp4p1/3Q4 w - - 0 1",
              6,
              {{33, 822, 26875, 678449, 22195229, 569451518}});
}

TEST_CASE("Move generation test 234", "[move-gen][case-234]")
{
  doPerftTest("7b/k4P1p/p2P2R1/3N3n/8/rR1K3P/2N2P1P/7B b - - 0 1",
              6,
              {{20, 786, 15702, 646012, 13128951, 547569692}});
}

TEST_CASE("Move generation test 235", "[move-gen][case-235]")
{
  doPerftTest("1r2n3/3KP3/4P1p1/3P4/2k3n1/P3Qp2/bp3p2/1R3N2 w - - 0 1",
              6,
              {{29, 495, 11756, 238892, 6001270, 136642458}});
}

TEST_CASE("Move generation test 236", "[move-gen][case-236]")
{
  doPerftTest("6N1/3QRP1K/6p1/2p2P2/Pn3p2/b3p2b/3P2p1/6k1 w - - 0 1",
              6,
              {{34, 673, 24309, 506621, 18736798, 411435577}});
}

TEST_CASE("Move generation test 237", "[move-gen][case-237]")
{
  doPerftTest("4K2k/bn1p4/6BP/8/6n1/2PPB1Pr/1R3Rp1/6N1 b - - 0 1",
              6,
              {{24, 945, 21791, 825286, 19672555, 727419811}});
}

TEST_CASE("Move generation test 238", "[move-gen][case-238]")
{
  doPerftTest("5b2/pNkPpP2/P3P1PB/8/r6N/2r2B1R/6K1/8 w - - 0 1",
              6,
              {{33, 800, 24416, 583802, 18401067, 442716780}});
}

TEST_CASE("Move generation test 239", "[move-gen][case-239]")
{
  doPerftTest("8/1K2P1nP/Bp3BbP/p6R/8/2P1k1b1/5NP1/4N3 b - - 0 1",
              6,
              {{26, 1078, 22363, 902377, 18394265, 736039317}});
}

TEST_CASE("Move generation test 240", "[move-gen][case-240]")
{
  doPerftTest("1B4Q1/2p3qP/1nP1P2P/3P4/R4r1P/8/4k2K/b2n4 w - - 0 1",
              6,
              {{31, 1536, 44882, 1990917, 58974644, 2441607194}});
}

TEST_CASE("Move generation test 241", "[move-gen][case-241]")
{
  doPerftTest("5k2/1n2R3/4P3/3p2PK/p1P2P1P/1b1NpP2/p6b/3r4 w - - 0 1",
              6,
              {{21, 537, 11087, 282075, 5801006, 149989639}});
}

TEST_CASE("Move generation test 242", "[move-gen][case-242]")
{
  doPerftTest("1R6/2Ppr3/B1pP4/4n1K1/p1p1N3/3n3P/1Pr3P1/6k1 w - - 0 1",
              6,
              {{37, 1115, 33774, 1021814, 29425825, 903371258}});
}

TEST_CASE("Move generation test 243", "[move-gen][case-243]")
{
  doPerftTest("R7/2P5/bpnK4/2B4p/4Prp1/PQ1pbP2/8/5kr1 b - - 0 1",
              6,
              {{36, 1350, 45356, 1613062, 54928229, 1904987712}});
}

TEST_CASE("Move generation test 244", "[move-gen][case-244]")
{
  doPerftTest("4K3/Q3PpRn/1p6/2p3pp/1k4Pq/3P4/2RnPP2/8 b - - 0 1",
              6,
              {{21, 663, 12897, 418493, 8864871, 293198874}});
}

TEST_CASE("Move generation test 245", "[move-gen][case-245]")
{
  doPerftTest("5r2/pPP5/4PP2/p1k5/2p1qp2/1b5K/1B1P1R2/1B5R b - - 0 1",
              6,
              {{41, 1230, 42528, 1349460, 44400904, 1463606761}});
}

TEST_CASE("Move generation test 246", "[move-gen][case-246]")
{
  doPerftTest("1k1r4/8/p2r2p1/1n1P4/2P2P2/n1b1PqRR/P3P2K/8 b - - 0 1",
              6,
              {{48, 742, 35180, 638114, 30015197, 574905055}});
}

TEST_CASE("Move generation test 247", "[move-gen][case-247]")
{
  doPerftTest("b4n2/8/1R1P1p1B/1p4P1/P3P3/2k3pK/1q1pB3/1r1Q4 w - - 0 1",
              6,
              {{33, 753, 25540, 673655, 22619946, 656870071}});
}

TEST_CASE("Move generation test 248", "[move-gen][case-248]")
{
  doPerftTest("1r6/2PP3p/R4p2/b7/2b4N/k3K2P/1p3p1p/2R3Br b - - 0 1",
              6,
              {{51, 1695, 73320, 2438352, 100149969, 3322730202}});
}

TEST_CASE("Move generation test 249", "[move-gen][case-249]")
{
  doPerftTest("n2Bb1R1/1R1P4/4Q3/8/1P3bPp/p1k5/3qBP2/N6K b - - 0 1",
              6,
              {{32, 1496, 42930, 1995193, 57758483, 2669622523}});
}

TEST_CASE("Move generation test 250", "[move-gen][case-250]")
{
  doPerftTest("1K6/3p1P2/2p1b3/2b2kp1/B3p2P/5p2/PQPp4/2q3r1 w - - 0 1",
              6,
              {{31, 1160, 32116, 1217166, 33185388, 1265968149}});
}

TEST_CASE("Move generation test 251", "[move-gen][case-251]")
{
  doPerftTest("K2B4/1P3P1p/8/4pR2/1n1PP1P1/bQNP4/6p1/2N3rk b - - 0 1",
              6,
              {{16, 775, 15068, 713202, 15059436, 706867310}});
}

TEST_CASE("Move generation test 252", "[move-gen][case-252]")
{
  doPerftTest("8/2BP3p/1P1PRP2/3bp3/pPP3K1/1p4P1/1p2rp2/1k6 w - - 0 1",
              6,
              {{19, 489, 8705, 232527, 4420497, 123222183}});
}

TEST_CASE("Move generation test 253", "[move-gen][case-253]")
{
  doPerftTest("n5r1/k4NBp/4P3/r3qN2/2P3P1/R5pK/2p3n1/3b3B w - - 0 1",
              6,
              {{33, 1525, 47830, 2162805, 67013821, 3037455843}});
}

TEST_CASE("Move generation test 254", "[move-gen][case-254]")
{
  doPerftTest("r1r4Q/P2p4/4P3/1pb3Pp/1q3nP1/8/pR1P3K/2k4n w - - 0 1",
              6,
              {{25, 1029, 25562, 1082313, 27876324, 1203955565}});
}

TEST_CASE("Move generation test 255", "[move-gen][case-255]")
{
  doPerftTest("1n6/2kP2K1/n4pP1/1P3qrp/p1b3R1/2p1pp2/Q7/2r5 w - - 0 1",
              6,
              {{32, 1291, 38046, 1591512, 46267260, 1962927092}});
}

TEST_CASE("Move generation test 256", "[move-gen][case-256]")
{
  doPerftTest("3Rn3/p2Pk3/B3P2P/2P2P1p/p2r2Qb/8/5Bnp/2K5 b - - 0 1",
              6,
              {{31, 1088, 27543, 975843, 24858443, 886579520}});
}

TEST_CASE("Move generation test 257", "[move-gen][case-257]")
{
  doPerftTest("R5r1/K4nb1/PP1Rp2p/3p1p2/1kPPp2P/4P2q/8/8 b - - 0 1",
              6,
              {{36, 542, 19577, 328604, 11831703, 208592832}});
}

TEST_CASE("Move generation test 258", "[move-gen][case-258]")
{
  doPerftTest("1r1B4/PPpk1n2/1b5P/3P4/2N1RQ2/3N3K/p6p/5Rr1 b - - 0 1",
              6,
              {{36, 2037, 62116, 3494734, 109202721, 6077697819}});
}

TEST_CASE("Move generation test 259", "[move-gen][case-259]")
{
  doPerftTest("4b3/2n2pK1/1p5P/2p3RP/1PN1ppp1/pqB5/8/nk6 w - - 0 1",
              6,
              {{29, 836, 22255, 676576, 17532830, 546990853}});
}

TEST_CASE("Move generation test 260", "[move-gen][case-260]")
{
  doPerftTest("3R3B/4n1PP/1P1Q4/r2b2P1/2N1kP2/1K4bP/p3P3/2R5 w - - 0 1",
              6,
              {{43, 1140, 44935, 1199723, 48767532, 1308085807}});
}

TEST_CASE("Move generation test 261", "[move-gen][case-261]")
{
  doPerftTest("6K1/1qprP1n1/8/2Q4P/1ppp1P2/3P4/NP1p1r2/n5Nk w - - 0 1",
              6,
              {{33, 1229, 36539, 1417555, 42050124, 1676236650}});
}

TEST_CASE("Move generation test 262", "[move-gen][case-262]")
{
  doPerftTest("1R5r/2k3P1/3n4/2b1q3/P3pp2/pN6/1B1KNP2/1B1bnr2 b - - 0 1",
              6,
              {{55, 1919, 97182, 3231295, 159241668, 5188053182}});
}

TEST_CASE("Move generation test 263", "[move-gen][case-263]")
{
  doPerftTest("2B4b/1rP1p1Pb/1ppNKp1r/1P2p3/8/6k1/1P5R/2B1q3 b - - 0 1",
              6,
              {{37, 1221, 39026, 1234800, 40450990, 1263879481}});
}

TEST_CASE("Move generation test 264", "[move-gen][case-264]")
{
  doPerftTest("2K1bRnr/b3P3/3Q4/BP2p3/7p/p1PNp3/1P1N1p2/k7 b - - 0 1",
              6,
              {{27, 1268, 34506, 1544195, 44141299, 1909041771}});
}

TEST_CASE("Move generation test 265", "[move-gen][case-265]")
{
  doPerftTest("Q3b3/p7/p3R2K/pp3P2/3P4/BP1Pn3/N3P1P1/bk4r1 b - - 0 1",
              6,
              {{26, 917, 23296, 834603, 21600512, 775061080}});
}

TEST_CASE("Move generation test 266", "[move-gen][case-266]")
{
  doPerftTest("1k6/1pr1P2p/1Np4r/Rn2P1pP/1p2p1n1/7p/7q/K5B1 w - - 0 1",
              6,
              {{24, 641, 13861, 414610, 9179093, 294371977}});
}

TEST_CASE("Move generation test 267", "[move-gen][case-267]")
{
  doPerftTest("K6B/4P1pB/1Pnn1p2/1b1p2Q1/1p2P3/1r1p3k/1P1RP3/8 b - - 0 1",
              6,
              {{27, 809, 15764, 489091, 10151317, 326698469}});
}

TEST_CASE("Move generation test 268", "[move-gen][case-268]")
{
  doPerftTest("q7/2p3p1/1R3p2/BNKQ4/5Pp1/p5Pn/1nP3rr/1k3b2 w - - 0 1",
              6,
              {{45, 1673, 57325, 2156023, 70882145, 2723904886}});
}

TEST_CASE("Move generation test 269", "[move-gen][case-269]")
{
  doPerftTest("R2Q1Kb1/1P1P4/1rpk3P/1B4N1/p3p3/P4P2/1rP4q/2B5 b - - 0 1",
              6,
              {{37, 1400, 40689, 1509062, 43518852, 1612354056}});
}

TEST_CASE("Move generation test 270", "[move-gen][case-270]")
{
  doPerftTest("Q4B1k/1RP5/3R2K1/N6n/r2Ppb1N/qr6/p2PP1B1/8 b - - 0 1",
              6,
              {{38, 1547, 56919, 2440740, 91741548, 4044959980}});
}

TEST_CASE("Move generation test 271", "[move-gen][case-271]")
{
  doPerftTest("3n3R/1P5N/R2K3P/1k2p3/4p1p1/PB1n1qpP/2P4P/6Bb w - - 0 1",
              6,
              {{41, 918, 33011, 793763, 27979702, 692518080}});
}

TEST_CASE("Move generation test 272", "[move-gen][case-272]")
{
  doPerftTest("8/N1p2p1P/QP3k1p/1PBp4/1K1NB3/2PP1p2/p1Pr4/b7 b - - 0 1",
              6,
              {{17, 517, 8215, 263642, 4512964, 149666182}});
}

TEST_CASE("Move generation test 273", "[move-gen][case-273]")
{
  doPerftTest("k1n4q/8/n1P5/2N2ppR/B3PbrP/p2K1p2/2PBp1Q1/8 b - - 0 1",
              6,
              {{44, 1294, 56222, 1702477, 73163486, 2258164397}});
}

TEST_CASE("Move generation test 274", "[move-gen][case-274]")
{
  doPerftTest("3K4/5p2/4rP2/BpP1nbp1/3p2Q1/2P1k1p1/pRp2q2/4r2N b - - 0 1",
              6,
              {{49, 1290, 49873, 1321038, 52402476, 1399777165}});
}

TEST_CASE("Move generation test 275", "[move-gen][case-275]")
{
  doPerftTest("1N1kr3/pQ1Bb3/p1P1K3/P3R3/br2p1P1/P5N1/3pp2p/8 w - - 0 1",
              6,
              {{30, 894, 22986, 727697, 19450978, 638061560}});
}

TEST_CASE("Move generation test 276", "[move-gen][case-276]")
{
  doPerftTest("1r3b2/P1p1P3/3pKp2/Pr5R/p7/qp3p1R/PN1kp3/7Q w - - 0 1",
              6,
              {{49, 1557, 69188, 2172899, 95672650, 3042227885}});
}

TEST_CASE("Move generation test 277", "[move-gen][case-277]")
{
  doPerftTest("3B4/2KP4/QP2k2p/6R1/pp3P2/2pq1P2/2Npr3/NRb4r b - - 0 1",
              6,
              {{40, 1491, 50818, 1852992, 63099800, 2310550329}});
}

TEST_CASE("Move generation test 278", "[move-gen][case-278]")
{
  doPerftTest("6k1/1P4bb/2K4n/p3rQpR/PR3P2/P2rB1PB/4P1p1/8 b - - 0 1",
              6,
              {{36, 1393, 44041, 1725875, 55966170, 2210239752}});
}

TEST_CASE("Move generation test 279", "[move-gen][case-279]")
{
  doPerftTest("nNk2r2/8/pR1P1pK1/1q2QP2/1P1pn2P/1P2B3/N5Pp/8 w - - 0 1",
              6,
              {{37, 1160, 39357, 1206722, 40331515, 1236902433}});
}

TEST_CASE("Move generation test 280", "[move-gen][case-280]")
{
  doPerftTest("1Q3Nr1/2N1PP1q/2K3B1/2pP1bP1/3Pk3/b5R1/n2pp3/B7 b - - 0 1",
              6,
              {{32, 1414, 41914, 1834319, 56828735, 2454721065}});
}

TEST_CASE("Move generation test 281", "[move-gen][case-281]")
{
  doPerftTest("3r4/B1b2Pkp/n5q1/PNP3P1/2R1np2/Pb2p1R1/2P1K3/6r1 w - - 0 1",
              6,
              {{26, 1418, 34257, 1789987, 44952763, 2277850700}});
}

TEST_CASE("Move generation test 282", "[move-gen][case-282]")
{
  doPerftTest("2Bb4/2P1pP2/7K/p3p2Q/PR2P1p1/R3Pp2/3kP3/N3qbn1 b - - 0 1",
              6,
              {{22, 912, 17201, 714425, 14275119, 595876337}});
}

TEST_CASE("Move generation test 283", "[move-gen][case-283]")
{
  doPerftTest("8/1B1p4/7K/1P2pRP1/1ppPpp2/2rPP1kn/1P4q1/4Rb1n w - - 0 1",
              6,
              {{30, 940, 28060, 865938, 25335857, 774336181}});
}

TEST_CASE("Move generation test 284", "[move-gen][case-284]")
{
  doPerftTest("3n4/pk5r/1P6/2pb1pKP/1P3P2/2b1pqpp/5BnP/1BN5 w - - 0 1",
              6,
              {{20, 1060, 18050, 904699, 15597602, 755956689}});
}

TEST_CASE("Move generation test 285", "[move-gen][case-285]")
{
  doPerftTest("1K4B1/N1Q1P3/rb2b2P/N1qk4/pr2n1RP/B6R/2P2p1p/8 w - - 0 1",
              6,
              {{50, 1553, 69068, 2330792, 102516831, 3642822302}});
}

TEST_CASE("Move generation test 286", "[move-gen][case-286]")
{
  doPerftTest("k1KN4/3rN1bp/P1P3pP/r4B1P/4b2p/2R3p1/P3p1q1/6R1 b - - 0 1",
              6,
              {{50, 1749, 86088, 2914656, 141959559, 4706917659}});
}

TEST_CASE("Move generation test 287", "[move-gen][case-287]")
{
  doPerftTest("1N3KN1/1r3b2/1QpR3p/2b4B/PP3p1p/pP1Pp3/P3P2k/8 w - - 0 1",
              6,
              {{24, 598, 15062, 370391, 10139955, 244454929}});
}

TEST_CASE("Move generation test 288", "[move-gen][case-288]")
{
  doPerftTest("2r4n/6q1/5p2/3pRp2/KPn3B1/1P1PP1p1/k2p1P1B/2b2b1Q b - - 0 1",
              6,
              {{51, 1319, 64983, 1685066, 80660510, 2134666495}});
}

TEST_CASE("Move generation test 289", "[move-gen][case-289]")
{
  doPerftTest("6Q1/P1BKP1R1/p3rpb1/2k4P/p1P3Nn/1Pp2p2/n5pq/8 w - - 0 1",
              6,
              {{43, 1589, 58610, 2083917, 75049964, 2634892225}});
}

TEST_CASE("Move generation test 290", "[move-gen][case-290]")
{
  doPerftTest("6q1/Pp2N2p/P3R2P/2p2pB1/2Pr3p/3p2R1/4BN2/bk2Kn2 w - - 0 1",
              6,
              {{46, 1612, 67755, 2379191, 97061529, 3397600891}});
}

TEST_CASE("Move generation test 291", "[move-gen][case-291]")
{
  doPerftTest("1B1rr3/pBP1P3/3pn1P1/1N1P1K2/pPk5/6RQ/1p1pPP2/2q5 w - - 0 1",
              6,
              {{52, 1582, 66578, 2108332, 86789724, 2825867335}});
}

TEST_CASE("Move generation test 292", "[move-gen][case-292]")
{
  doPerftTest("1K4b1/1p2bp1Q/NP1NqP2/1P3pRp/3p1p1r/2k5/5pBP/1n6 w - - 0 1",
              6,
              {{39, 1328, 46068, 1603552, 54972180, 1947527057}});
}

TEST_CASE("Move generation test 293", "[move-gen][case-293]")
{
  doPerftTest("1R6/n2pp3/1PpPkp2/1NB1b2r/p2PPp2/p2Pp3/K5P1/4q3 b - - 0 1",
              6,
              {{34, 641, 19066, 371225, 10978001, 221658728}});
}

TEST_CASE("Move generation test 294", "[move-gen][case-294]")
{
  doPerftTest("6R1/1NK1p3/2PP3k/PRPp2p1/8/P2qpB2/NbQ1bnPp/8 b - - 0 1",
              6,
              {{40, 1817, 66746, 2950971, 105858992, 4570425913}});
}

TEST_CASE("Move generation test 295", "[move-gen][case-295]")
{
  doPerftTest("R7/1Bp3K1/1b1P4/2PPRpp1/r2P2Nk/3p2p1/1Nn1Pq2/r6n b - - 0 1",
              6,
              {{45, 1937, 82030, 3324819, 136387906, 5304143538}});
}

TEST_CASE("Move generation test 296", "[move-gen][case-296]")
{
  doPerftTest("2nB4/5P1N/p1p2pb1/P3p2P/Pb1p2P1/2k1PP1p/7P/n4K1R b - - 0 1",
              6,
              {{31, 635, 19098, 394500, 11347765, 242428453}});
}

TEST_CASE("Move generation test 297", "[move-gen][case-297]")
{
  doPerftTest("6n1/1Pr5/q3k1bP/1p6/B1P1p1r1/1p1p1Q1p/P1nK2pp/R3N3 w - - 0 1",
              6,
              {{36, 1727, 59207, 2798502, 95375075, 4435595303}});
}

TEST_CASE("Move generation test 298", "[move-gen][case-298]")
{
  doPerftTest("5N1B/1k6/4Prp1/1bpK1pp1/4rpP1/1BP1P3/P1p1pN1R/R7 b - - 0 1",
              6,
              {{36, 1105, 38859, 1199499, 42148993, 1311010839}});
}

TEST_CASE("Move generation test 299", "[move-gen][case-299]")
{
  doPerftTest("4BB1q/n1k2p1P/3NP2P/1Pn3R1/1r1RNpbb/2p1P3/2K2p2/8 b - - 0 1",
              6,
              {{46, 1642, 70005, 2487654, 104371226, 3712283829}});
}

TEST_CASE("Move generation test 300", "[move-gen][case-300]")
{
  doPerftTest("r7/2r1NbP1/2p4K/PNpp4/1pPRBp2/p4Pqk/2P4b/6R1 b - - 0 1",
              6,
              {{38, 1212, 43192, 1385117, 49247954, 1597405203}});
}

TEST_CASE("Move generation test 301", "[move-gen][case-301]")
{
  doPerftTest("8/3P3P/RB3P1P/p1p1p1p1/3kp2N/2pp2K1/br1P2P1/1n1N1q1B w - - 0 1",
              6,
              {{28, 739, 18281, 498202, 12836811, 355953699}});
}

TEST_CASE("Move generation test 302", "[move-gen][case-302]")
{
  doPerftTest("1Q6/P2b1PNp/K3P3/1Bp1PP2/1Pp1p3/1p2rPRN/3kpn2/1n6 b - - 0 1",
              6,
              {{31, 1250, 36667, 1486706, 42800934, 1751694836}});
}

TEST_CASE("Move generation test 303", "[move-gen][case-303]")
{
  doPerftTest("1n1RBNr1/5Q1P/2pp1pP1/2pP2P1/2k3P1/2r2PPb/8/b3KnN1 b - - 0 1",
              6,
              {{28, 868, 24508, 755717, 21659876, 674299339}});
}

TEST_CASE("Move generation test 304", "[move-gen][case-304]")
{
  doPerftTest("3qn3/ppQ1p2B/1P2bB1p/7k/K4b2/1P1ppR1P/2rNRn2/4N3 b - - 0 1",
              6,
              {{48, 2004, 87916, 3602762, 152864101, 6215821574}});
}

TEST_CASE("Move generation test 305", "[move-gen][case-305]")
{
  doPerftTest("r7/3pQbnq/1p1PP3/3P1r1n/2Bk1P2/p5Rp/P1PNK2P/4N3 w - - 0 1",
              6,
              {{40, 1102, 39300, 1084728, 38064496, 1078402958}});
}

TEST_CASE("Move generation test 306", "[move-gen][case-306]")
{
  doPerftTest("3q4/1pP1NP2/1B3pBp/1k1P1RP1/4P1p1/1K1p2bP/2PN3P/1n1n4 w - - 0 1",
              6,
              {{45, 1400, 57097, 1709745, 68323689, 2015327472}});
}

TEST_CASE("Move generation test 307", "[move-gen][case-307]")
{
  doPerftTest("b5K1/1n5N/P3B1R1/3RPp2/P1pPpP2/4r1P1/p1P1p1k1/3qQ2n b - - 0 1",
              6,
              {{28, 1029, 26581, 992736, 27686163, 1043694792}});
}

TEST_CASE("Move generation test 308", "[move-gen][case-308]")
{
  doPerftTest("3Q2nK/p2nP2P/pb2P2P/2p1P3/bN2kB2/2r5/1pPP1p2/3r3R w - - 0 1",
              6,
              {{40, 1661, 66031, 2623742, 103751215, 3995489031}});
}

TEST_CASE("Move generation test 309", "[move-gen][case-309]")
{
  doPerftTest("3N2bn/1q1R3R/1P1r3p/1BP2K1k/1P2pP2/P2p4/ppP2Pp1/n7 w - - 0 1",
              6,
              {{28, 1034, 25851, 941774, 24140554, 887723350}});
}

TEST_CASE("Move generation test 310", "[move-gen][case-310]")
{
  doPerftTest("1B6/1kp4p/7P/1nQ1bnN1/1pP1KPR1/2pP1pr1/P1P1ppR1/8 b - - 0 1",
              6,
              {{35, 1086, 31338, 986445, 28910839, 918507621}});
}

TEST_CASE("Move generation test 311", "[move-gen][case-311]")
{
  doPerftTest("2r2k2/P3pBqp/1B3P1p/3K1PRp/b7/3p2bp/R2P1NNn/5r1n b - - 0 1",
              6,
              {{48, 1671, 73437, 2546642, 109133966, 3754752793}});
}

TEST_CASE("Move generation test 312", "[move-gen][case-312]")
{
  doPerftTest("4N3/2P4k/r1pKnP2/B2pPP1n/pP4p1/2b2B2/1Rqpp1b1/4N2R w - - 0 1",
              6,
              {{30, 1258, 37297, 1527954, 46626726, 1897617442}});
}

TEST_CASE("Move generation test 313", "[move-gen][case-313]")
{
  doPerftTest("6n1/3Kp3/1qpb1P2/2P3Qb/pRRP2rP/2Pk2p1/P4PpN/r6n b - - 0 1",
              6,
              {{46, 1309, 53348, 1551361, 62964314, 1864728073}});
}

TEST_CASE("Move generation test 314", "[move-gen][case-314]")
{
  doPerftTest("7K/2rn1N1Q/p1b3N1/1p2P1pP/B1n1R1P1/1pr1P3/pBkp1p1P/8 b - - 0 1",
              6,
              {{41, 1048, 43571, 1198433, 50029728, 1460351976}});
}

TEST_CASE("Move generation test 315", "[move-gen][case-315]")
{
  doPerftTest("1N6/1Bp1P1PQ/Pr2k2P/2b2NK1/Pp3pR1/pr2PPp1/7p/R5n1 b - - 0 1",
              6,
              {{27, 978, 22248, 824289, 19588431, 743203204}});
}

TEST_CASE("Move generation test 316", "[move-gen][case-316]")
{
  doPerftTest("1B5N/P3PPP1/Pbb4k/P1n1p3/pp3pR1/3qP3/P1p2KQ1/2r1R3 b - - 0 1",
              6,
              {{50, 2040, 77680, 3200962, 115953276, 4855771341}});
}

TEST_CASE("Move generation test 317", "[move-gen][case-317]")
{
  doPerftTest("k2b4/Nr1P2PN/K5p1/nn2P1P1/q1p1PP1B/p5Q1/1R2R1pr/5B2 b - - 0 1",
              6,
              {{36, 1182, 43968, 1522387, 58484670, 2106783684}});
}

TEST_CASE("Move generation test 318", "[move-gen][case-318]")
{
  doPerftTest("1K4Bk/2P1R3/4Pp1p/1Pn1P2p/PB1bP3/2p2r2/P1pRQ2p/2q2N2 w - - 0 1",
              6,
              {{38, 1350, 51602, 1814305, 72407158, 2538632245}});
}

TEST_CASE("Move generation test 319", "[move-gen][case-319]")
{
  doPerftTest("2r1n3/1P1N1PRR/q7/1P1b1B1p/PK4n1/1pP1kPp1/2Pb1p1B/2r5 b - - 0 1",
              6,
              {{58, 2186, 116271, 4297573, 219207045, 8051877028}});
}

TEST_CASE("Move generation test 320", "[move-gen][case-320]")
{
  doPerftTest("3K4/r1Pn1NPb/k2p2B1/1n1pp3/3qRPrN/2P3PP/2PB1p2/b7 b - - 0 1",
              6,
              {{39, 1354, 52645, 1832034, 74130760, 2572605139}});
}

TEST_CASE("Move generation test 321", "[move-gen][case-321]")
{
  doPerftTest("2n1K2k/N3ppR1/Np1P2PP/Q4b1p/4P1p1/3p4/nqR1BPPP/4B3 b - - 0 1",
              6,
              {{34, 1609, 50189, 2300834, 69984951, 3153767572}});
}

TEST_CASE("Move generation test 322", "[move-gen][case-322]")
{
  doPerftTest("5Rn1/prn1pP2/2NpPb1P/1kP5/3QPN1P/qpR3P1/pp5P/4K3 b - - 0 1",
              6,
              {{33, 1585, 47745, 2217378, 68282464, 3092512256}});
}

TEST_CASE("Move generation test 323", "[move-gen][case-323]")
{
  doPerftTest("2k5/Prpr2pp/2pPn2P/4P3/1QR1P3/3pKN1b/NR1p2pq/Bn6 b - - 0 1",
              6,
              {{38, 1233, 45680, 1527612, 59395869, 2020930904}});
}

TEST_CASE("Move generation test 324", "[move-gen][case-324]")
{
  doPerftTest("1r2N3/1Bppp2p/b6P/3KnR2/Pb1B1P2/PQ3ppr/N1n3PP/5k2 w - - 0 1",
              6,
              {{44, 1618, 61578, 2231082, 83626374, 3012474448}});
}

TEST_CASE("Move generation test 325", "[move-gen][case-325]")
{
  doPerftTest("6n1/1Rr1bP2/1B5N/1PP1kp1P/P1r1N2p/4pppp/2P3P1/1B1b1Q1K w - - 0 1",
              6,
              {{39, 1229, 44863, 1341767, 49404746, 1432757919}});
}

TEST_CASE("Move generation test 326", "[move-gen][case-326]")
{
  doPerftTest("6B1/1kp2rP1/4NpQB/1K1P3p/1pP1b2p/2PP1np1/3NP1qb/1r2R3 w - - 0 1",
              6,
              {{42, 1463, 56060, 1962396, 74669589, 2637548948}});
}

TEST_CASE("Move generation test 327", "[move-gen][case-327]")
{
  doPerftTest("3r1R1Q/PNP2bpp/PBpp1n1q/4PK1p/3P2P1/p7/3N2P1/nB1k4 w - - 0 1",
              6,
              {{41, 1491, 50603, 1788093, 58672181, 2054816862}});
}

TEST_CASE("Move generation test 328", "[move-gen][case-328]")
{
  doPerftTest("bB5n/RQrP1rPk/B1P4p/KN6/1P5p/1N5p/Ppn2pPp/b7 w - - 0 1",
              6,
              {{35, 1122, 40732, 1415134, 53259952, 1944511927}});
}

TEST_CASE("Move generation test 329", "[move-gen][case-329]")
{
  doPerftTest("3K2nR/P2bp3/PrPQ4/kp4PP/Pp2RrB1/5p2/1B1N1Pn1/4bN2 w - - 0 1",
              6,
              {{59, 1577, 82131, 2211020, 112018317, 3073980621}});
}

TEST_CASE("Move generation test 330", "[move-gen][case-330]")
{
  doPerftTest("2r1B3/1pRK1P1R/2P1p2q/1pb4p/5P2/2p2r1Q/pkP1NPbB/N2n4 w - - 0 1",
              6,
              {{28, 1133, 32728, 1313323, 40602732, 1632149010}});
}

TEST_CASE("Move generation test 331", "[move-gen][case-331]")
{
  doPerftTest("1q3b2/Pp2ppR1/PRP3B1/4N3/1P1pPP1r/kn1p1P2/1n2p2P/N1Q3K1 b - - 0 1",
              6,
              {{35, 1360, 44595, 1701909, 55970151, 2131165770}});
}

TEST_CASE("Move generation test 332", "[move-gen][case-332]")
{
  doPerftTest("1R2q3/PKpPp2b/N2BR1NP/P1b1Q2n/B1kppP1P/2p5/1p5r/7r w - - 0 1",
              6,
              {{46, 1789, 76131, 2923975, 126423665, 4797500304}});
}

TEST_CASE("Move generation test 333", "[move-gen][case-333]")
{
  doPerftTest("6k1/p1P1R3/1pKP1p1p/2p2pq1/3r2pP/N1p1PPBP/1R1PPnQ1/3b4 w - - 0 1",
              6,
              {{45, 1454, 61008, 1901498, 78223378, 2389170741}});
}

TEST_CASE("Move generation test 334", "[move-gen][case-334]")
{
  doPerftTest("3RB3/3n1KP1/1Rpp1N2/Nn3p2/PP3p2/ppPPk1P1/P1r1b3/b2QB3 w - - 0 1",
              6,
              {{43, 1042, 42013, 1011172, 41299732, 999581636}});
}

TEST_CASE("Move generation test 335", "[move-gen][case-335]")
{
  doPerftTest("7r/2pP1R1n/1P3Pq1/P1p1p2B/Qb1k2p1/PP4Kb/2p1p1Rp/n5NN w - - 0 1",
              6,
              {{27, 943, 25758, 925332, 27008395, 988481107}});
}

TEST_CASE("Move generation test 336", "[move-gen][case-336]")
{
  doPerftTest("4B1Rn/b1kp2pp/5pn1/1p2PQ2/N2RrP1P/2K1P3/1p2pNPP/6qb w - - 0 1",
              6,
              {{45, 1850, 68506, 2770172, 100896460, 4052774408}});
}

TEST_CASE("Move generation test 337", "[move-gen][case-337]")
{
  doPerftTest("3r4/1Kb4b/3qPppP/1N1Rp1Pk/1p3p1B/PPp1N1P1/p2nr3/1n3B2 b - - 0 1",
              6,
              {{44, 1079, 45130, 1147785, 48413822, 1262258229}});
}

TEST_CASE("Move generation test 338", "[move-gen][case-338]")
{
  doPerftTest("3k2bq/RP1pp3/p2P1B1p/1PpN3K/1RPnB3/3p1Pr1/nP3b1N/7Q w - - 0 1",
              6,
              {{41, 1255, 47860, 1512701, 57473011, 1861454770}});
}

TEST_CASE("Move generation test 339", "[move-gen][case-339]")
{
  doPerftTest("6BR/np5K/prk1P3/2P3pN/1Rp2N2/2PpPnpP/1p2P2p/3r1Q1q w - - 0 1",
              6,
              {{30, 857, 26483, 781964, 24676766, 759392699}});
}

TEST_CASE("Move generation test 340", "[move-gen][case-340]")
{
  doPerftTest("8/1nN1p1K1/ppbr2RP/p2p2Q1/Pp1n1p2/2PrP3/1P1Pq2R/bN3k2 w - - 0 1",
              6,
              {{42, 1438, 56322, 1978648, 77176306, 2765166822}});
}

TEST_CASE("Move generation test 341", "[move-gen][case-341]")
{
  doPerftTest("1b3B2/p1p1PnK1/k3P3/2pP2Pb/2p2P1q/p1PPr1p1/1pBP1N2/r1nR4 b - - 0 1",
              6,
              {{43, 1097, 46919, 1258863, 53269527, 1472940255}});
}

TEST_CASE("Move generation test 342", "[move-gen][case-342]")
{
  doPerftTest("1k1B4/3PpRp1/4pppn/pb6/nPPqPPPK/2pNP3/rPp1B3/5r2 b - - 0 1",
              6,
              {{47, 1170, 51804, 1299982, 55849208, 1413483335}});
}

TEST_CASE("Move generation test 343", "[move-gen][case-343]")
{
  doPerftTest("b3K3/7r/P1p3PB/1R1pPpn1/2ppNPpP/3n1B2/P1Rb1QPP/6qk b - - 0 1",
              6,
              {{47, 2138, 85793, 3770126, 141802239, 6070604492}});
}

TEST_CASE("Move generation test 344", "[move-gen][case-344]")
{
  doPerftTest("8/pKpN2PP/2P2n2/1p2B1pQ/b2RP1np/rB1PP1p1/1krbppN1/8 b - - 0 1",
              6,
              {{43, 1981, 77857, 3539783, 140485138, 6348231446}});
}

TEST_CASE("Move generation test 345", "[move-gen][case-345]")
{
  doPerftTest("2b4Q/1Ppp2PR/p3pb1K/1N1p2n1/qnP5/P2r1pk1/1P2R1PN/1rBB4 w - - 0 1",
              6,
              {{47, 1940, 86758, 3507647, 157040038, 6274224206}});
}

TEST_CASE("Move generation test 346", "[move-gen][case-346]")
{
  doPerftTest("2n1N3/r2b2pR/BP1p1PK1/N3Q3/3pRB1q/Ppnp4/prpb1PP1/3k4 b - - 0 1",
              6,
              {{49, 1839, 85170, 3344111, 152520084, 6199424322}});
}

TEST_CASE("Move generation test 347", "[move-gen][case-347]")
{
  doPerftTest("r7/PPpPpp2/3B2kr/bnnPK3/1PpQ1pq1/pR1b1B2/2pN1N1p/8 w - - 0 1",
              6,
              {{47, 2634, 103650, 5655409, 222373449, 11886378222}});
}

TEST_CASE("Move generation test 348", "[move-gen][case-348]")
{
  doPerftTest("6b1/P1b2P1p/1PQP2p1/1Pp1R1N1/Kpkr2p1/P3P1n1/1BBp2pr/1n4R1 b - - 0 1",
              6,
              {{31, 1749, 48531, 2640241, 75461645, 4013977245}});
}

TEST_CASE("Move generation test 349", "[move-gen][case-349]")
{
  doPerftTest("6RB/p3b3/p3PPp1/1RpPp2N/k1Kpp1B1/r5r1/2P1PPPN/1Q2q2b b - - 0 1",
              6,
              {{37, 1269, 39297, 1380116, 40866023, 1464467628}});
}

TEST_CASE("Move generation test 350", "[move-gen][case-350]")
{
  doPerftTest("6Nb/1Q1pP1P1/p2k1P1p/n4p1p/P1R1Pp2/np2p3/2BPK1RP/rb2B3 w - - 0 1",
              6,
              {{54, 774, 37584, 617779, 28121202, 506144400}});
}

TEST_CASE("Move generation test 351", "[move-gen][case-351]")
{
  doPerftTest("1k6/p1pn1Pb1/R2rR3/2PP2BP/B1P2npP/1q2N1pp/r1P1P2p/1b3KN1 w - - 0 1",
              6,
              {{38, 1914, 65946, 3198642, 107089203, 5049564365}});
}

TEST_CASE("Move generation test 352", "[move-gen][case-352]")
{
  doPerftTest("4B3/3pN1p1/p3P1p1/2ppKP1P/rnQPn2b/q1PPR2P/Rp2p3/2k3Br w - - 0 1",
              6,
              {{32, 1303, 39627, 1542418, 48669758, 1858185810}});
}

TEST_CASE("Move generation test 353", "[move-gen][case-353]")
{
  doPerftTest("r1N2q2/2b1p1QP/1PP2n1P/pn1k2P1/1P2RB2/RNppp3/rp1pB3/3b3K b - - 0 1",
              6,
              {{45, 2168, 88209, 4138298, 167124157, 7671320607}});
}

TEST_CASE("Move generation test 354", "[move-gen][case-354]")
{
  doPerftTest("r3N2Q/1pPr4/pnpR1PNP/P5bP/2P1n3/pB1K4/RpPPpk1p/2q5 w - - 0 1",
              6,
              {{28, 1544, 42204, 2236899, 64324380, 3298764472}});
}

TEST_CASE("Move generation test 355", "[move-gen][case-355]")
{
  doPerftTest("R5rn/ppp3pR/PQ2KbN1/PBP4p/4N1nP/1p1bBqp1/1p2P1P1/4k3 w - - 0 1",
              6,
              {{49, 2090, 88220, 3726913, 150206359, 6308106821}});
}

TEST_CASE("Move generation test 356", "[move-gen][case-356]")
{
  doPerftTest("4n3/1ppp4/PP2PpP1/2Rp1PP1/rB1k3q/ppbBN1Pb/1Nn1p1R1/6K1 b - - 0 1",
              6,
              {{39, 1415, 50248, 1752990, 63059215, 2157379469}});
}

TEST_CASE("Move generation test 357", "[move-gen][case-357]")
{
  doPerftTest("8/rP1pRnPN/Prb2Pp1/N3pp1q/2K5/k1pPPBP1/1pp1n1P1/2b4Q w - - 0 1",
              6,
              {{36, 1599, 50777, 2246900, 73334922, 3243726137}});
}

TEST_CASE("Move generation test 358", "[move-gen][case-358]")
{
  doPerftTest("1b4r1/PqP4B/Ppr2N2/Pp2QbpP/p3R2P/1Pp1BK2/knN1pP2/1n6 w - - 0 1",
              6,
              {{64, 2759, 148932, 6483382, 333177060, 14607576468}});
}

TEST_CASE("Move generation test 359", "[move-gen][case-359]")
{
  doPerftTest("4BB2/1rpnpP1q/ppR1P1P1/b2p4/2PKbNkN/2rpn3/pP1RP2p/8 b - - 0 1",
              6,
              {{47, 1209, 56130, 1505114, 68710514, 1903978428}});
}

TEST_CASE("Move generation test 360", "[move-gen][case-360]")
{
  doPerftTest("n4q2/1Ppp1R2/nPB1bp2/PQPP3P/P6p/1Pp1k3/pNK1pr1B/b5N1 b - - 0 1",
              6,
              {{40, 1276, 43168, 1387861, 46347270, 1521178065}});
}

TEST_CASE("Move generation test 361", "[move-gen][case-361]")
{
  doPerftTest("k3b3/B1pR1Bp1/4pP1p/pQK2N2/1PPP2P1/N2p1R1r/pPp3PP/b1r2q2 w - - 0 1",
              6,
              {{43, 1172, 48291, 1382645, 55094121, 1649527402}});
}

TEST_CASE("Move generation test 362", "[move-gen][case-362]")
{
  doPerftTest("1r2n1N1/pp1p2kB/P2rPR1q/2P1PP1P/1N1BpRp1/K5Q1/p2nb1pp/4b3 b - - 0 1",
              6,
              {{50, 1942, 89840, 3372859, 157635873, 5837369704}});
}

TEST_CASE("Move generation test 363", "[move-gen][case-363]")
{
  doPerftTest("7n/NK2pP2/1p1PR1n1/bPPp1Ppk/2pP3p/rbPq3p/2Q1pBP1/R5r1 b - - 0 1",
              6,
              {{41, 1984, 77895, 3580061, 140147213, 6195548679}});
}

TEST_CASE("Move generation test 364", "[move-gen][case-364]")
{
  doPerftTest("1n1q3N/P1R2b2/Pb2pB2/pppP4/P1r1PpKR/3QP3/p2rPNPp/4k2n b - - 0 1",
              6,
              {{46, 2185, 94988, 4335912, 185698828, 8272722747}});
}

TEST_CASE("Move generation test 365", "[move-gen][case-365]")
{
  doPerftTest("B7/2P1pRpp/NR1r2Pp/nbP1bp2/2k1P3/n1P1KP1p/pBp1PQ1r/7N b - - 0 1",
              6,
              {{43, 1314, 53459, 1703179, 68229918, 2244599832}});
}

TEST_CASE("Move generation test 366", "[move-gen][case-366]")
{
  doPerftTest("2QBn1rb/3pPpP1/n3p1p1/1KNP1kP1/1ppq4/PR1P2Pr/2NpP3/1R5B w - - 0 1",
              6,
              {{50, 2131, 98259, 3979415, 182977123, 7220303818}});
}

TEST_CASE("Move generation test 367", "[move-gen][case-367]")
{
  doPerftTest("B5n1/2N5/Rq1rP1Pp/rpp1bk1P/P1P1ppbB/QpRp4/3n1PPP/4K3 w - - 0 1",
              6,
              {{34, 1197, 40345, 1483807, 50646941, 1908753604}});
}

TEST_CASE("Move generation test 368", "[move-gen][case-368]")
{
  doPerftTest("2B1R3/6Pp/1pP4p/B1N1b2Q/pr2nrpp/4pRqP/PNp1PP2/Kb3kn1 w - - 0 1",
              6,
              {{43, 1943, 83504, 3746434, 163479269, 7322130574}});
}

TEST_CASE("Move generation test 369", "[move-gen][case-369]")
{
  doPerftTest("2N3R1/1ppPp1rP/n2rBPP1/2bbpRPQ/1P2pBpP/2N4p/2q3P1/K3k3 b - - 0 1",
              6,
              {{46, 1665, 73483, 2631837, 115377566, 4136802105}});
}

TEST_CASE("Move generation test 370", "[move-gen][case-370]")
{
  doPerftTest("q2R2r1/kNpPrP2/pp2N1PQ/3bpP2/pp1bB3/Pp2nKPP/5BP1/3R4 w - - 0 1",
              6,
              {{53, 1886, 93054, 3225738, 154188511, 5230843124}});
}

TEST_CASE("Move generation test 371", "[move-gen][case-371]")
{
  doPerftTest("3n4/PqP4K/n1pBbkpP/2p2P2/rpPN1p1R/P1pRr2P/1pN3Bp/2Q1b3 b - - 0 1",
              6,
              {{50, 2121, 96857, 4235435, 186713682, 8352243708}});
}

TEST_CASE("Move generation test 372", "[move-gen][case-372]")
{
  doPerftTest("1Q2B1R1/r4q1P/pN1P2b1/PpKn2p1/1R1P1Pp1/1p1k1pn1/1Bp2PPP/r1b3N1 b - - 0 1",
              6,
              {{49, 2019, 92506, 3697641, 165224065, 6492038815}});
}

TEST_CASE("Move generation test 373", "[move-gen][case-373]")
{
  doPerftTest("2rB1n2/1PP2B1p/2qp2pp/1Q1NpRbR/n4PNP/1rPPpbpP/4k2p/K7 b - - 0 1",
              6,
              {{50, 2055, 98470, 3994558, 188446466, 7574357494}});
}

TEST_CASE("Move generation test 374", "[move-gen][case-374]")
{
  doPerftTest("1Q3bn1/PprpPRP1/1Rp5/k2b1P1p/3rqp2/N1pP2pP/2PP1B1n/3BN1K1 b - - 0 1",
              6,
              {{38, 1601, 57598, 2412894, 87958318, 3709063022}});
}

TEST_CASE("Move generation test 375", "[move-gen][case-375]")
{
  doPerftTest("7R/P3pnrP/1P1pNq1p/2P1prBP/pP1Nn1P1/2kp1Rpp/1b4b1/1B1K3Q w - - 0 1",
              6,
              {{51, 1461, 67600, 2067508, 93411929, 3013657972}});
}

TEST_CASE("Move generation test 376", "[move-gen][case-376]")
{
  doPerftTest("8/2pp1N1P/1b2nPBP/nK1P3P/ppP2pr1/R2pqb1P/NB1RPp1p/2r2k2 b - - 0 1",
              6,
              {{50, 1712, 83261, 2806926, 135586374, 4550952647}});
}

TEST_CASE("Move generation test 377", "[move-gen][case-377]")
{
  doPerftTest("rNn2Q2/1qN3p1/pb1p3P/Pp3P1p/1PbR1PPB/pPK3nk/p5pr/2RB4 b - - 0 1",
              6,
              {{44, 1685, 73550, 2741052, 119893129, 4436053561}});
}

TEST_CASE("Move generation test 378", "[move-gen][case-378]")
{
  doPerftTest("1K1b2rR/Np1B4/1n1Ppr1Q/q1ppRP1p/PPbP1PNP/2Bk1p1p/5p1n/8 w - - 0 1",
              6,
              {{37, 1363, 46676, 1752994, 60734140, 2328721510}});
}

TEST_CASE("Move generation test 389", "[move-gen][case-379]")
{
  doPerftTest("1rb2Q1b/3NpP2/Pp4p1/3B2B1/1nNpP3/1KP2PPp/pprRPPpk/R2n3q w - - 0 1",
              6,
              {{48, 1860, 81361, 3145684, 137268337, 5362679575}});
}

TEST_CASE("Move generation test 390", "[move-gen][case-380]")
{
  doPerftTest("n1K5/4kpqb/P2NP1p1/r1bRB3/1PpRPp2/p1PP1p1P/B1PpprN1/2n2Q2 b - - 0 1",
              6,
              {{42, 1247, 48452, 1453984, 55316778, 1705615762}});
}

TEST_CASE("Move generation test 381", "[move-gen][case-381]")
{
  doPerftTest("2rR1n2/1BP1kpp1/NBP1P1Pp/4qp1P/1bP3PR/1p1K1np1/pp1NrQ2/8 w - - 0 1",
              6,
              {{41, 2057, 67317, 3296490, 109116723, 5208141967}});
}

TEST_CASE("Move generation test 382", "[move-gen][case-382]")
{
  doPerftTest("nB2q3/1BPpPr2/Pk1n4/5pp1/1p2PNPP/N1rppQp1/R4pK1/1bb4R b - - 0 1",
              6,
              {{44, 1964, 79208, 3542355, 144933906, 6494854549}});
}

TEST_CASE("Move generation test 383", "[move-gen][case-383]")
{
  doPerftTest("2qnb1RK/P2PP3/1Br1p1pP/2b4R/pp1r1p2/1P1B4/PPp1np1P/kN4Q1 w - - 0 1",
              6,
              {{60, 2820, 157119, 7309581, 390573682, 18126084037}});
}

TEST_CASE("Move generation test 384", "[move-gen][case-384]")
{
  doPerftTest("qnbr1bn1/1P2Br2/pPPN3N/p1p2R2/kPpPp3/2RP1P2/4PpBp/2K5 w - - 0 1",
              6,
              {{53, 1972, 88020, 3260964, 141383733, 5236661691}});
}

TEST_CASE("Move generation test 385", "[move-gen][case-385]")
{
  doPerftTest("2QR4/pb3NPp/n1N1pPPq/BBnp1R1r/1p3p2/b5kP/1PKpP1pr/8 b - - 0 1",
              6,
              {{37, 1639, 62640, 2785600, 110132391, 4883362303}});
}

TEST_CASE("Move generation test 386", "[move-gen][case-386]")
{
  doPerftTest("6N1/ppPpR1nq/1n3PN1/RPk1b1pP/3pQP1p/p3PPBB/3K2p1/3b1r2 w - - 0 1",
              6,
              {{46, 1640, 70565, 2533422, 107915249, 3897382314}});
}

TEST_CASE("Move generation test 387", "[move-gen][case-387]")
{
  doPerftTest("4Qn2/pBpq4/pp4p1/BPR2Pb1/bKP3P1/PPNPpp1n/2k2r2/R4r1N w - - 0 1",
              6,
              {{46, 2002, 80611, 3394981, 131215838, 5429394455}});
}

TEST_CASE("Move generation test 388", "[move-gen][case-388]")
{
  doPerftTest("R2r1B2/1b1P1p2/np2P3/rqp3PB/pp1pkb1P/p2NP1P1/p4PK1/n1QR4 w - - 0 1",
              6,
              {{48, 1465, 68360, 2100923, 97826688, 3069415602}});
}

TEST_CASE("Move generation test 389", "[move-gen][case-389]")
{
  doPerftTest("2b1KN2/5pR1/1P2BQp1/rp3n1P/2Np1npR/P1pp2PP/P1kpPb1q/2B5 w - - 0 1",
              6,
              {{40, 1862, 72024, 3223157, 126903252, 5528182267}});
}

TEST_CASE("Move generation test 390", "[move-gen][case-390]")
{
  doPerftTest("4r3/2N1nBp1/P2Q2bp/P1p1P3/PBRNpp1r/1PKPp1n1/P4kp1/R5bq b - - 0 1",
              6,
              {{33, 1240, 40387, 1557977, 50696353, 2000811604}});
}

TEST_CASE("Move generation test 391", "[move-gen][case-391]")
{
  doPerftTest("5KN1/1k3P2/1P4RR/3ppr2/1PPBpPQ1/2PPq1pp/rpb1p1pP/bN1n1B2 b - - 0 1",
              6,
              {{50, 1798, 83710, 3183082, 141006918, 5529081305}});
}

TEST_CASE("Move generation test 392", "[move-gen][case-392]")
{
  doPerftTest("2rn1B2/2bNpB2/1pp3pP/p2RPPk1/2pP4/PbPP1p1q/r1Q4P/n1R1K2N b - - 0 1",
              6,
              {{36, 1155, 39678, 1257609, 43739452, 1390491178}});
}

TEST_CASE("Move generation test 393", "[move-gen][case-393]")
{
  doPerftTest("2K1B3/pPP1NP1k/2p1nBr1/2P1rn1b/1pPp2p1/1Rb1pPPP/2Nq1pR1/Q7 w - - 0 1",
              6,
              {{54, 2138, 104982, 4290510, 206437884, 8627404897}});
}

TEST_CASE("Move generation test 394", "[move-gen][case-394]")
{
  doPerftTest("2N5/np2PPnp/1brp4/pR1KP1P1/3BQ2N/1P1pP1pk/B1pPbP1r/4qR2 w - - 0 1",
              6,
              {{44, 1634, 71146, 2669710, 115711746, 4393554312}});
}

TEST_CASE("Move generation test 395", "[move-gen][case-395]")
{
  doPerftTest("3rbQ2/PP2p3/rbpPK2P/R2Rp3/1pn1n1p1/pq1PpNPk/p1P1B2P/B7 w - - 0 1",
              6,
              {{49, 1799, 74640, 2740538, 113032915, 4182937020}});
}

TEST_CASE("Move generation test 396", "[move-gen][case-396]")
{
  doPerftTest("8/p1Q2npP/bp1Np1Pr/RPPB1P2/P1RPpBp1/1q1p1n2/kP1pN3/4bK2 w - - 0 1",
              6,
              {{47, 2096, 93409, 3999850, 176530165, 7386025794}});
}

TEST_CASE("Move generation test 397", "[move-gen][case-397]")
{
  doPerftTest("2bK3k/bp1QNp2/r1Rn1ppP/1NP1PB2/2ppPPP1/1Pr2q1p/1P5p/R3B3 w - - 0 1",
              6,
              {{46, 1896, 83913, 3396531, 148506899, 5958571825}});
}

TEST_CASE("Move generation test 398", "[move-gen][case-398]")
{
  doPerftTest("1n1N4/rR1P1ppP/1R1Pp1n1/3Pp3/1NQpP2b/p4Br1/bP1KPPp1/k5Bq b - - 0 1",
              6,
              {{33, 1402, 44616, 1814739, 59750921, 2399676791}});
}

TEST_CASE("Move generation test 399", "[move-gen][case-399]")
{
  doPerftTest("6n1/3p4/BRP1bRr1/2Pp1pbP/N1P1PPp1/2p2pkp/KP2Qq1p/N1Br1n2 w - - 0 1",
              6,
              {{42, 1585, 63847, 2423979, 95050361, 3626792921}});
}

TEST_CASE("Move generation test 400", "[move-gen][case-400]")
{
  doPerftTest("1BBkb2n/1P4pP/PKP1p1pP/R3qNpP/3p3P/2P1p1np/N3rp1b/2rR4 b - - 0 1",
              6,
              {{42, 1280, 48851, 1528272, 59069836, 1903734536}});
}

TEST_CASE("Move generation test 401", "[move-gen][case-401]")
{
  doPerftTest("r1bq1rk1/pp2bpp1/2n1pn1p/2pp4/4P2B/P1NP1N2/1PP1BPPP/R2Q1RK1 b - - 3 9",
              6,
              {{34, 1054, 36722, 1179031, 41983813, 1394254859}});
}

TEST_CASE("Move generation test 402", "[move-gen][case-402]")
{
  doPerftTest("r1b1k1nr/ppp2ppp/1bnp1q2/4p3/PP2P2N/2PPB3/4BPPP/RN1Q1RK1 b kq - 0 11",
              6,
              {{40, 1308, 51595, 1680800, 65631311, 2174264835}});
}

TEST_CASE("Move generation test 403", "[move-gen][case-403]")
{
  doPerftTest("rnbqk2r/ppp2ppp/8/2bpp3/4P1n1/3PB1P1/PPP1NPBP/RN1QK2R b KQkq - 2 6",
              6,
              {{44, 1589, 68608, 2418543, 102879114, 3578858029}});
}

TEST_CASE("Move generation test 404", "[move-gen][case-404]")
{
  doPerftTest("r2qkb1r/1b1n1pp1/p2ppn1p/8/1p1NP1PP/2N1BP2/PPPQ4/2KR1B1R w kq - 0 12",
              6,
              {{41, 1426, 60093, 2114267, 90781021, 3228327782}});
}

TEST_CASE("Move generation test 405", "[move-gen][case-405]")
{
  doPerftTest("2rq1rk1/pp2ppbp/2npbnp1/8/4P3/2N3PP/PPP1NPB1/R1BQ1RK1 w - - 1 11",
              6,
              {{32, 1236, 42916, 1692326, 61995269, 2491030851}});
}

TEST_CASE("Move generation test 406", "[move-gen][case-406]")
{
  doPerftTest("r2q1rk1/ppp2pp1/1bnpbn1p/4p3/1PB1P3/2PP1N2/P2N1PPP/R1BQR1K1 w - - 3 10",
              6,
              {{32, 1273, 42378, 1660250, 57587949, 2234134001}});
}

TEST_CASE("Move generation test 407", "[move-gen][case-407]")
{
  doPerftTest("r1b1kb1r/pp1n1pp1/3p1q1p/2pP4/4Q1PP/2P1PPB1/P7/1R2KBNR b Kkq - 0 15",
              6,
              {{6, 283, 7097, 312771, 8584954, 360755551}});
}

TEST_CASE("Move generation test 408", "[move-gen][case-408]")
{
  doPerftTest("rn1qk2r/1p2bp2/p2pbBp1/4p2p/2B1P3/2N3NP/PPP2PP1/R2QK2R b KQkq - 0 11",
              6,
              {{30, 1480, 44274, 2080350, 63715655, 2899197539}});
}

TEST_CASE("Move generation test 409", "[move-gen][case-409]")
{
  doPerftTest("r2q1rk1/2p1bppp/p1npBn2/4p3/Pp2P3/3P1N2/1PPN1PPP/R1BQR1K1 b - - 0 11",
              6,
              {{27, 953, 25756, 899174, 25341781, 895278204}});
}

TEST_CASE("Move generation test 410", "[move-gen][case-410]")
{
  doPerftTest("rn1qk2r/pbp1bppp/1p2p3/1B1n4/3P4/P1N1PN2/1P3PPP/R1BQK2R b KQkq - 2 8",
              6,
              {{6, 250, 8779, 356821, 12882820, 515199510}});
}

TEST_CASE("Move generation test 411", "[move-gen][case-411]")
{
  doPerftTest("r1bqkb1r/1p3pp1/p2ppn1p/2n5/2BNP2B/2N5/PPP1QPPP/R3K2R w KQkq - 2 10",
              6,
              {{48, 1554, 72549, 2315265, 107815935, 3441288729}});
}

TEST_CASE("Move generation test 412", "[move-gen][case-412]")
{
  doPerftTest("r1b2rk1/2q1bppp/p2p1n2/npp1p3/3PP3/2P2N1P/PPBN1PP1/R1BQR1K1 b - - 2 12",
              6,
              {{38, 1175, 45440, 1478292, 57824130, 1970416797}});
}

TEST_CASE("Move generation test 413", "[move-gen][case-413]")
{
  doPerftTest("r1bq1rk1/3nbppp/2n1p3/2ppP3/pp3B1P/P2P1NP1/1PP2PB1/R2QRNK1 b - - 0 13",
              6,
              {{35, 1165, 41273, 1420368, 51121324, 1812997376}});
}

TEST_CASE("Move generation test 414", "[move-gen][case-414]")
{
  doPerftTest("r4rk1/b1p2p1n/2p5/p3B2q/3PP1b1/2P3R1/PP1N1P2/R2Q2K1 b - - 4 25",
              6,
              {{27, 977, 29307, 1046658, 34342519, 1218891295}});
}

TEST_CASE("Move generation test 415", "[move-gen][case-415]")
{
  doPerftTest("r3kb1r/p1q1n1pp/2p1bp2/2ppp2P/2P1P3/1P1P1N2/P4PP1/R1BQKN1R b KQkq - 0 11",
              6,
              {{36, 1300, 44977, 1612702, 54805859, 1972495307}});
}

TEST_CASE("Move generation test 416", "[move-gen][case-416]")
{
  doPerftTest("2rr2k1/3nqpbp/p2pp1p1/1P6/3BP3/1PN2P2/P3Q1PP/2RR2K1 b - - 0 19",
              6,
              {{37, 1465, 53372, 2145095, 78279068, 3182645102}});
}

TEST_CASE("Move generation test 417", "[move-gen][case-417]")
{
  doPerftTest("3r2k1/2p1b1pp/2n2pb1/3qP3/1p3BP1/2Pp3P/1P1N1P2/R2QR1K1 w - - 0 25",
              6,
              {{41, 1823, 72181, 3109569, 124226559, 5265268135}});
}

TEST_CASE("Move generation test 418", "[move-gen][case-418]")
{
  doPerftTest("r4rk1/3q1ppp/p1p1nn2/PpbPp3/8/2PPBQ1P/BP2NPP1/R4RK1 b - - 0 18",
              6,
              {{44, 1799, 76476, 3104878, 128043463, 5165612120}});
}

TEST_CASE("Move generation test 419", "[move-gen][case-419]")
{
  doPerftTest("r1bq1rk1/bpp2p2/2np1n1p/p3p1p1/2B1P3/2PP1NB1/PP1N1PPP/R2QR1K1 b - - 3 11",
              6,
              {{36, 1263, 45024, 1615351, 58763447, 2147172858}});
}

TEST_CASE("Move generation test 420", "[move-gen][case-420]")
{
  doPerftTest("r2qr1k1/4bpp1/p1npbn1p/1ppNp3/P3P3/2PP4/BP3PPP/R1BQRNK1 b - - 1 15",
              6,
              {{38, 1540, 57115, 2283790, 86080671, 3415595090}});
}
