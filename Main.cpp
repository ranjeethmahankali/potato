#include <Command.h>
#include <GLUtil.h>
#include <Position.h>
#include <Util.h>
#include <View.h>
#include <iostream>

using namespace potato;

static void gameLoop()
{
  std::string input;
  bool        running = true;
  while (running && !view::closed()) {
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

int main(int argc, char** argv)
{
  play();
  return 0;
}
