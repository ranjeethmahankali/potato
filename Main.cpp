#include <Command.h>
#include <GLUtil.h>
#include <Position.h>
#include <Util.h>
#include <View.h>
#include <argparse/argparse.hpp>
#include <iostream>

using namespace potato;

static void gameLoop()
{
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
}

static void play()
{
  command::init();
  view::start();
  gameLoop();
  view::stop();
  view::join();
}

static void cli()
{
  command::init();
  std::string input;
  bool        running = true;
  while (running) {
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
  parser.parse_args(argc, argv);
  if (parser["--cli"] == true) {
    cli();
  }
  else {
    play();
  }
  return 0;
}
