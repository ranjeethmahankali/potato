#include <Command.h>
#include <GLUtil.h>
#include <Position.h>
#include <Util.h>
#include <View.h>
#include <argparse/argparse.hpp>
#include <iostream>

using namespace potato;

static void play()
{
  view::start();
  view::join();
}

static void cli(const bool showBoard = true)
{
  command::init();
  if (showBoard) {
    view::start();
  }
  std::string input;
  bool        running = true;
  while (running && !view::closed()) {
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
  if (showBoard) {
    view::stop();
    view::join();
  }
}

int main(int argc, char** argv)
{
  argparse::ArgumentParser parser("potato");
  parser.add_argument("--cli")
    .help("Start in pure CLI mode instead of entering the game loop.")
    .implicit_value(true)
    .default_value(false);
  parser.add_argument("--no-board")
    .help("In CLI mode, don't show the board.")
    .implicit_value(true)
    .default_value(false);
  parser.parse_args(argc, argv);
  if (parser["--cli"] == true) {
    if (parser["--no-board"] == true) {
      cli(false);
    }
    else {
      cli();
    }
  }
  else {
    play();
  }
  return 0;
}
