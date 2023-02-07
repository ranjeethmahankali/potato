#include <Board.h>
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
    std::cout << "Received command: " << input << std::endl;
  }
};

static void advance(const Board& b)
{
  view::set(b);
  std::this_thread::sleep_for(std::chrono::seconds(1));
}

int main(int argc, char** argv)
{
  Board       b;
  std::thread cmdThread(cmdLoop);
  view::start();
  advance(b);
  advance(b.move({4, 6}, {4, 4}));
  advance(b.move({4, 1}, {4, 3}));
  advance(b.move({6, 7}, {5, 5}));
  advance(b.move({1, 0}, {2, 2}));
  advance(b.move({3, 6}, {3, 4}));
  advance(b.move({4, 3}, {3, 4}));
  advance(b.move({5, 5}, {3, 4}));
  advance(b.move({2, 2}, {3, 4}));
  advance(b.move({3, 7}, {3, 4}));
  cmdThread.join();
  view::stop();
  return 0;
}
