#pragma once

#include <spdlog/spdlog.h>

namespace potato {
namespace uci {

spdlog::logger& logger();

void start();

}  // namespace uci
}  // namespace potato
