#include <Board.h>
#include <GLUtil.h>
#include <View.h>
#include <chrono>
#include <iostream>
#include <thread>

using namespace potato;

int main(int argc, char** argv)
{
  RenderLoop& view = RenderLoop::start();
  Board       b;
  view.set(b);
  b.move({4, 1}, {4, 3});
  std::this_thread::sleep_for(std::chrono::seconds(2));
  view.set(b);
  b.move({4, 6}, {4, 4});
  std::this_thread::sleep_for(std::chrono::seconds(2));
  view.set(b);
  b.move({6, 0}, {5, 2});
  std::this_thread::sleep_for(std::chrono::seconds(2));
  view.set(b);
  b.move({2, 7}, {3, 5});
  std::this_thread::sleep_for(std::chrono::seconds(2));
  view.set(b);
  b.move({3, 1}, {3, 3});
  std::this_thread::sleep_for(std::chrono::seconds(2));
  return 0;
}
