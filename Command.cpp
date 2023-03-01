#include <ArgVSplit.h>
#include <Command.h>
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

Response doMove(const std::string& mv)
{
  // Generate the set of legal moves.
  MoveList legal;
  generateMoves(currentPosition(), legal);
  bool success = false;
  auto match   = std::find_if(
    legal.begin(), legal.end(), [&mv](const Move& m) { return m.algebraic() == mv; });
  if (match != legal.end()) {
    match->commit(currentPosition());
    view::update();
    success = true;
  }
  else if (mv.size() == 4) {  // Check for promotions
    StaticVector<char, 4> candidates;
    for (const auto& m : legal) {
      std::string alg = m.algebraic();
      if (alg.starts_with(mv)) {
        candidates.push_back(alg.back());
      }
    }
    if (!candidates.empty()) {
      std::cout << "Choose an option from ";
      for (char c : candidates) {
        std::cout << c << ", ";
      }
      std::cout << ": ";
      char response;
      std::cin >> response;
      std::string fullmove = mv;
      fullmove.push_back(response);
      match = std::find_if(legal.begin(), legal.end(), [&fullmove](const Move& m) {
        return m.algebraic() == fullmove;
      });
      if (match != legal.end()) {
        match->commit(currentPosition());
        view::update();
        success = true;
      }
    }
  }
  if (success) {
    return bestMove(currentPosition());
  }
  else {
    std::cout << "Not a legal move!\n";
    return Response::none();
  }
}

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

void move(int argc, const char** argv)
{
  argparse::ArgumentParser parser("move");
  parser.add_argument("movestr").help("The move").required();
  parser.parse_args(argc, argv);
  doMove(parser.get<std::string>("movestr"));
}

void loadFen(int argc, const char** argv)
{
  argparse::ArgumentParser parser("fen");
  parser.add_argument("fenstr")
    .help("The FEN string of the position to be loaded.")
    .required();
  parser.parse_args(argc, argv);
  auto fen          = parser.get<std::string>("fenstr");
  currentPosition() = Position::fromFen(fen);
  // This will update the view only if the view is actually open.
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

void show(int argc, const char** argv)
{
  std::cout << currentPosition() << std::endl
            << "FEN: " << currentPosition().fen() << std::endl;
}

}  // namespace funcs

void init()
{
  cmdFuncMap().emplace("move", funcs::move);
  cmdFuncMap().emplace("fen", funcs::loadFen);
  cmdFuncMap().emplace("perft", funcs::perft);
  cmdFuncMap().emplace("show", funcs::show);
}

}  // namespace command

}  // namespace potato
