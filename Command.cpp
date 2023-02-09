#include <Command.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <cxxopts.hpp>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace potato {

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
    logger().error("Unrecognized command {}", sArgV[0]);
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

  std::cout << "Received move: " << mv << std::endl;
  // TODO: Incomplete
}

}  // namespace funcs

void init()
{
  cmdFuncMap().emplace("move", funcs::move);
}

}  // namespace command

}  // namespace potato
