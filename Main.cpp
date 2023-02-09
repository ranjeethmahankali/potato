#include <Board.h>
#include <Command.h>
#include <GLUtil.h>
#include <View.h>
#include <chrono>
#include <iostream>
#include <thread>

using namespace potato;

static void cmdLoop()
{
  std::string input;
  bool        running = true;
  while (running) {
    std::cout << ">>> ";
    std::getline(std::cin, input);
    if (input.empty())
      continue;
    if (input == "exit") {
      running = false;
      continue;
    }
    command::run(input.begin(), input.end());
  }
};

int main(int argc, char** argv)
{
  command::init();
  Board       b;
  std::thread cmdThread(cmdLoop);
  view::start();
  cmdThread.join();
  view::stop();
  return 0;
}
