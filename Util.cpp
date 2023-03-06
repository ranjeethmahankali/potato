#include <Util.h>
#include <fstream>
#include <iostream>

#include <Position.h>

#ifdef WIN32
#include <Shlobj.h>
#define PATH_MAX MAX_PATH
#else
#include <linux/limits.h>
#include <unistd.h>
#endif

namespace potato {

fs::path absPath(const fs::path& relPath)
{
#ifdef WIN32
  wchar_t path[MAX_PATH];
  if (GetModuleFileNameW(0, path, MAX_PATH)) {
    return fs::path(path);
  }
  else {
    assert(0);
    return L"";
  }
#else
  std::string apath(PATH_MAX, '\0');
  ssize_t     count = readlink("/proc/self/exe", apath.data(), PATH_MAX);
  fs::path    path;
  if (count == -1) {
    throw "Cannot find absolute path";
  }
  apath.erase(apath.begin() + count, apath.end());
  path = fs::path(apath).parent_path() / relPath;
  return path;
#endif
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

Timer::Timer(const std::string& name)
    : mDesc(name)
    , mStart(InternalClockT::now())
{}

Timer::~Timer()
{
  auto elapsed = InternalClockT::now() - mStart;
  std::cout << mDesc << ": "
            << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count()
            << "ms\n";
}

}  // namespace potato
