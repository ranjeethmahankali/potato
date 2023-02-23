#pragma once

#include <chrono>
#include <filesystem>

namespace potato {

namespace fs = std::filesystem;

/**
 * @brief Get the absolute path.
 *
 * @param relPath Path relative to the executable
 * @return fs::path
 */
fs::path absPath(const fs::path& relPath);

/**
 * @brief Get the text inside the given file as a string.
 *
 * @param fpath
 * @return std::string
 */
std::string textFromFile(const fs::path& fpath);

/**
 * @brief RAII timer for measuring execution times.
 *
 */
class Timer
{
  using InternalClockT    = std::chrono::high_resolution_clock;
  using InternalDurationT = InternalClockT::duration;

public:
  /**
   * @brief Create a new timer that prints the elapsed time to the given output stream.
   *
   * @param name Name of the timer.
   * @param out Output stream.
   */
  explicit Timer(const std::string& name);
  ~Timer();

private:
  std::string                                    mName;
  std::chrono::high_resolution_clock::time_point mStart;
};

}  // namespace potato
