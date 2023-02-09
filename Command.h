#pragma once

#include <spdlog/spdlog.h>

namespace potato {

namespace command {

spdlog::logger& logger();

void init();

void run(std::string::iterator begin, std::string::iterator end);

}  // namespace command

}  // namespace potato
