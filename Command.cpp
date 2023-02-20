#include <ArgVSplit.h>
#include <Command.h>
#include <Move.h>
#include <Position.h>
#include <View.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <argparse/argparse.hpp>
#include <regex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace potato {

// bool doCastle(Position&                           b,
//               const std::string&                  mv,
//               const std::unordered_set<Position>& legal)
// {
//   static constexpr glm::ivec2 sKingPos   = {4, 7};
//   static constexpr glm::ivec2 sRookShort = {7, 7};
//   static constexpr glm::ivec2 sRookLong  = {0, 7};
//   if (mv == "o-o") {  // Short castle.
//     Position b2 = b;
//     if (legal.find(b2.move(sKingPos, {6, 7}).move(sRookShort, {5, 7})) != legal.end())
//     {
//       b = b2;
//       return true;
//     }
//   }
//   else if (mv == "o-o-o") {
//     Position b2 = b;
//     if (legal.find(b2.move(sKingPos, {2, 7}).move(sRookLong, {3, 7})) != legal.end()) {
//       b = b2;
//       return true;
//     }
//   }
//   return false;
// }

// static uint8_t charToPiece(char c)
// {
//   switch (c) {
//   case 'p':
//     return Piece::PWN;
//   case 'n':
//     return Piece::HRS;
//   case 'b':
//     return Piece::BSH;
//   case 'r':
//     return Piece::ROK;
//   case 'q':
//     return Piece::QEN;
//   case 'k':
//     return Piece::KNG;
//   default:
//     return Piece::NONE;
//   }
// }

// bool doPromotion(Position&                           b,
//                  const std::string&                  mv,
//                  const std::unordered_set<Position>& legal)
// {
//   std::smatch results;
//   if (std::regex_search(mv, results, std::regex("^([a-h])8=([n,b,r,q])$"))) {
//     int      file  = fileToX(*(results[1].first));
//     uint8_t  piece = charToPiece(*(results[2].first));
//     Position b2    = b;
//     if (legal.find(b2.move({file, 7}, {file, 8}).set({file, 8}, Piece::WHT | piece)) !=
//         legal.end()) {
//       b = b2;
//       return true;
//     }
//   }
//   else if (std::regex_search(mv, results,
//   std::regex("^([a-h])x([a-h])8=([n,b,r,q])$"))) {
//     int      file1 = fileToX(*(results[1].first));
//     int      file2 = fileToX(*(results[2].first));
//     uint8_t  piece = charToPiece(*(results[3].first));
//     Position b2    = b;
//     if (legal.find(b2.move({file1, 7}, {file2, 8}).set({file2, 8}, Piece::WHT | piece))
//     !=
//         legal.end()) {
//       b = b2;
//       return true;
//     }
//   }
//   return false;
// }

// bool doMove(Position& b, std::string mv)
// {
//   static std::vector<Position>        sLegalVec;
//   static std::unordered_set<Position> sLegalSet;
//   // Generate the set of legal moves.
//   sLegalVec.clear();
//   b.genMoves(sLegalVec, Piece::WHT);
//   sLegalSet.clear();
//   std::copy(
//     sLegalVec.begin(), sLegalVec.end(), std::inserter(sLegalSet, sLegalSet.end()));
//   // Lower the case.
//   for (auto& c : mv) {
//     c = std::tolower(c);
//   }
//   // Parse move.
//   if (doCastle(b, mv, sLegalSet)) {
//     return true;
//   }
//   std::smatch results;
//   if (std::regex_search(
//         mv,
//         results,
//         std::regex("^([p,n,b,r,q,k]{0,1})([a-h,1-8]{0,1})(x{0,1})([a-h])([1-8])$"))) {
//     uint8_t piece =
//       Piece::WHT | charToPiece(results[1].length() == 0 ? 'p' : *(results[1].first));
//     char disambiguation = results[2].length() == 0 ? 0 : *(results[2].first);
//     // Optionally, results[3] will contain the capture notation. It is not used.
//     int file = fileToX(*(results[4].first));
//     int rank = rankToY(*(results[5].first));
//     // Assume a file is specified to disambiguate - coord index 0.
//     int disCoordI = 0;
//     int dis       = fileToX(disambiguation);
//     if (dis == -1) {  // If not, assume it's the rank - coord index 1.
//       disCoordI = 1;
//       dis       = rankToY(disambiguation);
//     }
//     auto match = std::find_if(
//       sLegalVec.begin(),
//       sLegalVec.end(),
//       [piece, pos = glm::ivec2 {file, rank}, dis, disCoordI](const Position& b) {
//         uint8_t pc         = b.piece(pos);
//         bool    pieceMatch = Piece::color(pc) == Piece::color(piece) &&
//                           Piece::type(pc) == Piece::type(piece);
//         bool disMatch = dis == -1 || pos[disCoordI] == dis;
//         return disMatch && pieceMatch;
//       });
//     if (match == sLegalVec.end()) {
//       std::cout << "Not a legal move.\n";
//       return false;
//     }
//     b = *match;
//     return true;
//   }
//   if (doPromotion(b, mv, sLegalSet)) {
//     return true;
//   }
//   return false;
// }

namespace command {

using CmdFnPtr = void (*)(int, const char**);

spdlog::logger& logger()
{
  static auto sLogger = spdlog::stdout_color_mt("command");
  return *sLogger;
}

static std::unordered_map<std::string_view, CmdFnPtr>& cmdFuncMap()
{
  static std::unordered_map<std::string_view, CmdFnPtr> sCommandFnMap;
  return sCommandFnMap;
}

void run(const std::string& cmd)
{
  argv_split parser("");
  parser.parse(cmd);
  const char** argv  = parser.argv();
  int          argc  = int(parser.getArguments().size());
  auto         match = cmdFuncMap().find(argv[0]);
  if (match == cmdFuncMap().end()) {
    // If no command is specified, we expect a move.
    run("move " + cmd);
    return;
  }

  try {
    match->second(argc, argv);
  }
  catch (const std::exception& e) {
    logger().error("\n{}", e.what());
  }
}

namespace funcs {

// void move(int argc, const char** argv)
// {
//   static auto opts =
//     optionsWithPosnArgs<std::string>("move",
//                                      "Make a move on the board.",
//                                      {{"move",
//                                        "The move you want to play. For eample: rg1 will
//                                        " "move the rook to the g1 square."}});

//   auto parsed = parseOptions(argc, argv, opts);
//   if (!parsed) {
//     logger().error("Failed to parse command!");
//     return;
//   }
//   std::string mv = parsed.value()["move"].as<std::string>();
//   doMove(currentPosition(), mv);
//   view::update();
// }

void loadFen(int argc, const char** argv)
{
  argparse::ArgumentParser parser("fen");
  parser.add_argument("fenstr")
    .help("The FEN string of the position to be loaded.")
    .required();
  parser.parse_args(argc, argv);
  auto fen = parser.get<std::string>("fenstr");
  std::cout << "Received fen string: " << fen << std::endl;
  currentPosition() = Position::fromFen(fen);
  view::update();
}

void perft(int argc, const char** argv)
{
  argparse::ArgumentParser parser("perft");
  parser.add_argument("depth")
    .help("The depth to traverse when counting moves.")
    .required()
    .scan<'i', int>();
  parser.parse_args(argc, argv);
  int depth = parser.get<int>("depth");
  potato::perft(currentPosition(), depth);
}

}  // namespace funcs

void init()
{
  // cmdFuncMap().emplace("move", funcs::move);
  cmdFuncMap().emplace("fen", funcs::loadFen);
  cmdFuncMap().emplace("perft", funcs::perft);
}

}  // namespace command

}  // namespace potato
