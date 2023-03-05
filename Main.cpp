#include <Command.h>
#include <GLUtil.h>
#include <Position.h>
#include <Util.h>
#include <View.h>
#include <argparse/argparse.hpp>
#include <iostream>

using namespace potato;

static void play(bool asBlack = false, bool flipBoard = false)
{
  view::game(asBlack, flipBoard);
}

static void cli()
{
  command::init();
  std::string input;
  bool        running = true;
  while (running) {
    std::cout << ">>> ";
    std::getline(std::cin, input);
    if (input.empty())
      continue;
    if (input == "exit" || input == "quit") {
      running = false;
      continue;
    }
    command::run(input);
  }
}

int main(int argc, char** argv)
{
  argparse::ArgumentParser parser("potato");
  parser.add_argument("--cli")
    .help("Start in pure CLI mode instead of entering the game loop.")
    .implicit_value(true)
    .default_value(false);
  parser.add_argument("--as-black")
    .help("Start the game to play as black.")
    .implicit_value(true)
    .default_value(false);
  parser.add_argument("--flip-board")
    .help("See the board from Potato's side.")
    .implicit_value(true)
    .default_value(false);
  parser.parse_args(argc, argv);
  if (parser["--cli"] == true) {
    cli();
  }
  else {
    bool asBlack   = parser["--as-black"] == true;
    bool flipBoard = parser["--flip-board"] == true;
    play(asBlack, flipBoard);
  }

  return 0;
}
