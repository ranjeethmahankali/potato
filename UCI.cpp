#include <UCI.h>
#include <iostream>

namespace potato {
namespace uci {

static void run(const std::string& cmd)
{
  // TODO: Incomplete.
}

void start()
{
  std::string input;
  bool        running = true;
  while (running) {
    std::getline(std::cin, input);
    if (input.empty()) {
      continue;
    }
    if (input == "exit" || input == "quit") {
      running = false;
      continue;
    }
    run(input);
  }
}

}  // namespace uci
}  // namespace potato
