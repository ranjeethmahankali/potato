#include <Util.h>
#include <fstream>
#include <iostream>

#include <Position.h>
#include <linux/limits.h>
#include <unistd.h>

namespace potato {

fs::path absPath(const fs::path& relPath)
{
  std::string apath(PATH_MAX, '\0');
  ssize_t     count = readlink("/proc/self/exe", apath.data(), PATH_MAX);
  fs::path    path;
  if (count == -1) {
    throw "Cannot find absolute path";
  }
  apath.erase(apath.begin() + count, apath.end());
  path = fs::path(apath).parent_path() / relPath;
  return path;
}

std::string textFromFile(const fs::path& fpath)
{
  std::ifstream file;
  file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  std::stringstream file_stream;
  try {
    file.open(fpath);
    file_stream << file.rdbuf();
    file.close();
  }
  catch (const std::ifstream::failure& e) {
    std::cerr << "Error when reading file: " << fpath << std::endl;
  }
  return file_stream.str();
}

}  // namespace potato
