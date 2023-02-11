#pragma once

#include <spdlog/spdlog.h>

namespace potato {

namespace command {

spdlog::logger& logger();

void init();

void run(const std::string& cmd);

}  // namespace command

}  // namespace potato
