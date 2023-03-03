#pragma once

#include <Move.h>
#include <spdlog/spdlog.h>
#include <optional>

namespace potato {

namespace command {

spdlog::logger& logger();

void init();

void run(const std::string& cmd);

}  // namespace command

Response doMove(const std::string& mv);

}  // namespace potato
