#pragma once

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

}  // namespace potato
