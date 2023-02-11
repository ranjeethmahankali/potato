#include <Board.h>
#include <Command.h>
#include <View.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <cxxopts.hpp>
#include <regex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace potato {

bool doCastle(Board& b, const std::string& mv, const std::unordered_set<Board>& legal)
{
  static constexpr glm::ivec2 sKingPos   = {4, 7};
  static constexpr glm::ivec2 sRookShort = {7, 7};
  static constexpr glm::ivec2 sRookLong  = {0, 7};
  if (mv == "o-o") {  // Short castle.
    Board b2 = b;
    if (legal.find(b2.move(sKingPos, {6, 7}).move(sRookShort, {5, 7})) != legal.end()) {
      b = b2;
      return true;
    }
  }
  else if (mv == "o-o-o") {
    Board b2 = b;
    if (legal.find(b2.move(sKingPos, {2, 7}).move(sRookLong, {3, 7})) != legal.end()) {
      b = b2;
      return true;
    }
  }
  return false;
}

static int fileToX(char file)
{
  switch (file) {
  case 'a':
    return 0;
  case 'b':
    return 1;
  case 'c':
    return 2;
  case 'd':
    return 3;
  case 'e':
    return 4;
  case 'f':
    return 5;
  case 'g':
    return 6;
  case 'h':
    return 7;
  default:
    return -1;
  }
}

static int rankToY(char rank)
{
  switch (rank) {
  case '1':
    return 7;
  case '2':
    return 6;
  case '3':
    return 5;
  case '4':
    return 4;
  case '5':
    return 3;
  case '6':
    return 2;
  case '7':
    return 1;
  case '8':
    return 0;
  default:
    return -1;
  }
}

static uint8_t charToPiece(char c)
{
  switch (c) {
  case 'p':
    return Piece::PWN;
  case 'n':
    return Piece::HRS;
  case 'b':
    return Piece::BSH;
  case 'r':
    return Piece::ROK;
  case 'q':
    return Piece::QEN;
  case 'k':
    return Piece::KNG;
  default:
    return Piece::NONE;
  }
}

bool doPromotion(Board& b, const std::string& mv, const std::unordered_set<Board>& legal)
{
  std::smatch results;
  if (std::regex_search(mv, results, std::regex("^([a-h])8=([n,b,r,q])$"))) {
    int     file  = fileToX(*(results[1].first));
    uint8_t piece = charToPiece(*(results[2].first));
    Board   b2    = b;
    if (legal.find(
          b2.move({file, 7}, {file, 8}).setPiece({file, 8}, Piece::WHT | piece)) !=
        legal.end()) {
      b = b2;
      return true;
    }
  }
  else if (std::regex_search(mv, results, std::regex("^([a-h])x([a-h])8=([n,b,r,q])$"))) {
    int     file1 = fileToX(*(results[1].first));
    int     file2 = fileToX(*(results[2].first));
    uint8_t piece = charToPiece(*(results[3].first));
    Board   b2    = b;
    if (legal.find(
          b2.move({file1, 7}, {file2, 8}).setPiece({file2, 8}, Piece::WHT | piece)) !=
        legal.end()) {
      b = b2;
      return true;
    }
  }
  return false;
}

bool doMove(Board& b, std::string mv)
{
  static std::vector<Board>        sLegalVec;
  static std::unordered_set<Board> sLegalSet;
  // Generate the set of legal moves.
  sLegalVec.clear();
  b.genMoves(sLegalVec, Piece::WHT);
  sLegalSet.clear();
  std::copy(
    sLegalVec.begin(), sLegalVec.end(), std::inserter(sLegalSet, sLegalSet.end()));
  // Lower the case.
  for (auto& c : mv) {
    c = std::tolower(c);
  }
  // Parse move.
  if (doCastle(b, mv, sLegalSet)) {
    return true;
  }
  std::regex  pattern("^([p,n,b,r,q,k]{0,1})([a-h,1-8]{0,1})(x{0,1})([a-h])([1-8])$");
  std::smatch results;
  if (std::regex_search(mv, results, pattern)) {
    uint8_t piece =
      Piece::WHT | charToPiece(results[1].length() == 0 ? 'p' : *(results[1].first));
    char disambiguation = results[2].length() == 0 ? 0 : *(results[2].first);
    // Optionally, results[3] will contain the capture notation. It is not used.
    int file = fileToX(*(results[4].first));
    int rank = rankToY(*(results[5].first));
    // Assume a file is specified to disambiguate - coord index 0.
    int disCoordI = 0;
    int dis       = fileToX(disambiguation);
    if (dis == -1) {  // If not, assume it's the rank - coord index 1.
      disCoordI = 1;
      dis       = rankToY(disambiguation);
    }
    auto match = std::find_if(
      sLegalVec.begin(),
      sLegalVec.end(),
      [piece, pos = glm::ivec2 {file, rank}, dis, disCoordI](const Board& b) {
        uint8_t pc         = b.piece(pos);
        bool    pieceMatch = Piece::color(pc) == Piece::color(piece) &&
                          Piece::type(pc) == Piece::type(piece);
        bool disMatch = dis == -1 || pos[disCoordI] == dis;
        return disMatch && pieceMatch;
      });
    if (match == sLegalVec.end()) {
      std::cout << "Not a legal move.\n";
      return false;
    }
    b = *match;
    return true;
  }
  if (doPromotion(b, mv, sLegalSet)) {
    return true;
  }
  return false;
}

namespace command {

using CmdFnPtr = void (*)(int, char**);

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

void run(std::string::iterator begin, std::string::iterator end)
{
  static std::vector<char*> sArgV;
  sArgV.clear();
  for (auto it = begin; it != end; it++) {
    char& c = *it;
    if (c == ' ') {
      c = '\0';
    }
    else if (it == begin || *(it - 1) == '\0') {
      sArgV.push_back(&c);
    }
  }
  if (sArgV.empty()) {
    // No command received.
    return;
  }
  auto match = cmdFuncMap().find(sArgV[0]);
  if (match == cmdFuncMap().end()) {
    // If no command is specified, we expect a move.
    std::string newCmd = "move ";
    std::copy(begin, end, std::back_inserter(newCmd));
    run(newCmd.begin(), newCmd.end());
    return;
  }

  try {
    match->second(int(sArgV.size()), sArgV.data());
  }
  catch (const std::exception& e) {
    logger().error("\n{}", e.what());
  }
}

struct ArgDesc
{
  std::string_view name;
  std::string_view desc;
};

template<size_t I, typename... Ts>
void addOptions(cxxopts::OptionAdder& adder, const std::initializer_list<ArgDesc>& args)
{
  using Type = std::tuple_element_t<I, std::tuple<Ts...>>;

  if constexpr (I < sizeof...(Ts)) {
    const auto& arg = *(args.begin() + I);
    adder(arg.name.data(), arg.desc.data(), cxxopts::value<Type>(), arg.name.data());
  }

  if constexpr (I + 1 < sizeof...(Ts)) {
    addOptions<I + 1, Ts...>(adder, args);
  }
}

cxxopts::Options optionsWithoutArgs(const std::string_view& name,
                                    const std::string_view& desc)
{
  cxxopts::Options opts(name.data(), desc.data());
  opts.allow_unrecognised_options().add_options()("help", "Print help");
  return opts;
}

template<typename... Ts>
cxxopts::Options optionsWithPosnArgs(const std::string_view&               name,
                                     const std::string_view&               desc,
                                     const std::initializer_list<ArgDesc>& args)
{
  if (!(sizeof...(Ts) == 1 || args.size() == sizeof...(Ts))) {
    logger().error("Incorrect number of argument types for the options parser.");
  }
  auto opts  = optionsWithoutArgs(name, desc);
  auto adder = opts.add_options();
  addOptions<0, Ts...>(adder, args);
  std::vector<std::string> argNames(args.size());
  std::transform(args.begin(), args.end(), argNames.begin(), [](const auto& pair) {
    return pair.name;
  });
  opts.parse_positional(argNames);
  return opts;
}

std::optional<cxxopts::ParseResult> parseOptions(int               argc,
                                                 char**            argv,
                                                 cxxopts::Options& opts)
{
  auto parsed = opts.parse(argc, argv);
  if (parsed.count("help")) {
    logger().info(opts.help());
    return std::nullopt;
  }
  return parsed;
}

namespace funcs {

void move(int argc, char** argv)
{
  static auto opts =
    optionsWithPosnArgs<std::string>("move",
                                     "Make a move on the board.",
                                     {{"move",
                                       "The move you want to play. For eample: rg1 will "
                                       "move the rook to the g1 square."}});

  auto parsed = parseOptions(argc, argv, opts);
  if (!parsed) {
    logger().error("Failed to parse command!");
    return;
  }
  std::string mv = parsed.value()["move"].as<std::string>();
  doMove(currentBoard(), mv);
  view::update();
}

}  // namespace funcs

void init()
{
  cmdFuncMap().emplace("move", funcs::move);
}

}  // namespace command

}  // namespace potato
