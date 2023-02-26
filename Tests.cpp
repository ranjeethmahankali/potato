#define CATCH_CONFIG_MAIN
#include <Move.h>
#include <Util.h>
#include <algorithm>
#include <bit>
#include <catch.hpp>
#include <catch2/catch_all.hpp>
#include <iostream>
#include <stack>

using namespace potato;

void doPerftTest(const std::string&      fenstr,
                 size_t                  depth,
                 std::span<const size_t> expected);

// TEST_CASE("Debugging", "[debug]")
// {
//   // doPerftTest("rnbqk2r/p1ppppbp/1p3np1/8/3P4/1P3N2/PBP1PPPP/RN1QKB1R w KQkq - 0 5",
//   //             6,
//   //             {{28, 753, 21943, 618608, 18819585, 550948819}});
//   Position p =
//     Position::fromFen("2rr2k1/3nqpbp/p2pp1p1/1P6/3BP3/1PN2P2/P3Q1PP/2RR2K1 b - - 0
//     19");
//   perft(p, 5);
// }

TEST_CASE("Fen Consistency", "[fen][consistency]")
{
  SECTION("Case 1")
  {
    Position p =
      Position::fromFen("rnbqkbnr/1ppppppp/p7/P7/8/8/1PPPPPPP/RNBQKBNR b KQkq - 0 2");
    Move(DBL_PUSH, B7, B5).commit(p);
    REQUIRE(p.fen() == "rnbqkbnr/2pppppp/p7/Pp6/8/8/1PPPPPPP/RNBQKBNR w KQkq b6 0 3");
  }
  SECTION("Case 2")
  {
    Position p =
      Position::fromFen("rnbqkbnr/pppp1ppp/4p3/8/5P2/5N2/PPPPP1PP/RNBQKB1R b KQkq - 1 2");
    Move(OTHER, D8, H4).commit(p);
    REQUIRE(p.fen() == "rnb1kbnr/pppp1ppp/4p3/8/5P1q/5N2/PPPPP1PP/RNBQKB1R w KQkq - 2 3");
  }
  SECTION("Case 3")
  {
    Position p = Position::fromFen(
      "r1bqk2r/ppp2ppp/2n5/1B1pp3/3Pn3/b1N2N2/PPP2PPP/R1BQ1RK1 b kq - 3 7");
    Move(MV_KNG, E8, D7).commit(p);
    REQUIRE(p.fen() ==
            "r1bq3r/pppk1ppp/2n5/1B1pp3/3Pn3/b1N2N2/PPP2PPP/R1BQ1RK1 w - - 4 8");
  }
  SECTION("Case 4")
  {
    Position p = Position::fromFen(
      "r1bqk2r/ppppbp1p/8/3nB1p1/2B1P3/3P4/PPP2PPP/RN2K1NR w KQkq - 0 8");
    Move(OTHER | CAPTURE, E5, H8).commit(p);
    REQUIRE(p.fen() == "r1bqk2B/ppppbp1p/8/3n2p1/2B1P3/3P4/PPP2PPP/RN2K1NR b KQq - 0 8");
  }
  SECTION("Case 5")
  {
    Position p = Position::fromFen(
      "r1bqkb1r/1pp2ppp/2n5/pB1pp3/3Pn3/5N2/PPP2PPP/RNBQ1RK1 w kq - 0 7");
    Move(OTHER | CAPTURE, B5, C6).commit(p);
    REQUIRE(p.fen() == "r1bqkb1r/1pp2ppp/2B5/p2pp3/3Pn3/5N2/PPP2PPP/RNBQ1RK1 b kq - 0 7");
  }
  SECTION("Case 6")
  {
    Position p = Position::fromFen(
      "r1bqkb1r/ppp2ppp/2n5/1B1pp3/3Pn3/5N2/PPP2PPP/RNBQ1RK1 b kq - 1 6");
    Move(MV_ROK, A8, B8).commit(p);
    REQUIRE(p.fen() == "1rbqkb1r/ppp2ppp/2n5/1B1pp3/3Pn3/5N2/PPP2PPP/RNBQ1RK1 w k - 2 7");
  }
  SECTION("Case 7")
  {
    Position p =
      Position::fromFen("1nq1n3/1P3b1p/p2p2kp/6P1/4pKp1/rP2r3/2N5/1B6 w - - 1 2");
    Move(CAPTURE | PRC_BSH, B7, C8).commit(p);
    REQUIRE(p.fen() == "1nB1n3/5b1p/p2p2kp/6P1/4pKp1/rP2r3/2N5/1B6 b - - 0 2");
  }
  SECTION("Case 8")
  {
    Position p = Position::fromFen(
      "rnbqk2r/2ppppbp/1p3np1/p7/1P1P4/P4N2/1BP1PPPP/RN1QKB1R b KQkq - 0 6");
    Move(CAPTURE | OTHER, A5, B4).commit(p);
    REQUIRE(p.fen() ==
            "rnbqk2r/2ppppbp/1p3np1/8/1p1P4/P4N2/1BP1PPPP/RN1QKB1R w KQkq - 0 7");
  }
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

TEST_CASE("Material count", "[material][value][incremental]")
{
  SECTION("Starting position")
  {
    Position p;
    REQUIRE(p.material() == 0);
    REQUIRE(p.valid());
  }
  SECTION("Pawn captures by white")
  {
    Position p = Position::fromFen(
      "r1b2rk1/pp1nqppp/3bpn2/2p5/2BP4/2N1PN1P/PPQ2PP1/R1B2RK1 w - - 0 11");
    REQUIRE(p.valid());
    REQUIRE(p.material() == 0);
    Move(OTHER | CAPTURE, D4, C5).commit(p);
    REQUIRE(p.valid());
    REQUIRE(p.material() == 1);
    Move(OTHER | CAPTURE, D6, C5).commit(p);
    REQUIRE(p.valid());
    REQUIRE(p.material() == 0);
  }
  SECTION("Piece captures by white")
  {
    Position p = Position::fromFen(
      "r1r3k1/1b1nqppp/p2bp3/1p2n3/3BP3/PBN2N1P/1P3PP1/1Q1RR1K1 w - - 10 31");
    REQUIRE(p.valid());
    REQUIRE(p.material() == 0);
    Move(OTHER | CAPTURE, F3, E5).commit(p);
    REQUIRE(p.valid());
    REQUIRE(p.material() == 3);
    Move(OTHER | CAPTURE, D7, E5).commit(p);
    REQUIRE(p.valid());
    REQUIRE(p.material() == 0);
  }
  SECTION("Pawn captures by black")
  {
    Position p =
      Position::fromFen("r5k1/1b3ppp/p3p3/1pr5/4PP1q/PB1Q3P/1P1nN1P1/2R1R1K1 b - - 3 37");
    REQUIRE(p.valid());
    REQUIRE(p.material() == 0);
    Move(OTHER | CAPTURE, D2, E4).commit(p);
    REQUIRE(p.valid());
    REQUIRE(p.material() == -1);
  }
  SECTION("Uneven trades")
  {
    Position p =
      Position::fromFen("5k1r/5p2/3pbb2/qp1B4/1Nr4p/P2RQ3/2P3PP/1K1R4 w - - 3 29");
    REQUIRE(p.valid());
    REQUIRE(p.material() == 0);
    Move(OTHER | CAPTURE, D5, C4).commit(p);
    REQUIRE(p.valid());
    REQUIRE(p.material() == 5);
    Move(OTHER | CAPTURE, B5, C4).commit(p);
    REQUIRE(p.valid());
    REQUIRE(p.material() == 2);
  }
}

TEST_CASE("Best Move", "[bestmove]")
{
  Position p =
    Position::fromFen("rnbqkbnr/1ppppppp/B7/8/4P3/8/PPPP1PPP/RNBQK1NR b KQkq - 0 2");
  Move m = bestMove(p);
  REQUIRE(m.algebraic() == "b7a6");
}
