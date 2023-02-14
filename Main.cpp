#include <Command.h>
#include <GLUtil.h>
#include <Position.h>
#include <View.h>
#include <chrono>
#include <iostream>
#include <thread>
#include "Move.h"

#include <Util.h>

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
    if (input == "exit") {
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
  currentPosition() =
    Position::fromFen("r1bqk2r/ppp2ppp/2n2n2/4p3/4p3/2P2NP1/PPP2PBP/R1BQ1RK1 w kq - 0 8");
  std::cout << currentPosition() << std::endl;
  // play();
  return 0;
}
