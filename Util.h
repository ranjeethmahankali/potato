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
   * @param description Of what is being timed.
   * @param out Output stream.
   */
  explicit Timer(const std::string& description);
  ~Timer();

private:
  std::string                                    mDesc;
  std::chrono::high_resolution_clock::time_point mStart;
};

template<typename T, size_t N>
struct StaticVector
{
  static constexpr size_t Size = N;
  using value_type             = T;

protected:
  std::array<T, N> mBuf;
  T*               mEnd;

public:
  StaticVector()
      : mBuf()
      , mEnd(mBuf.data())
  {}
  StaticVector(size_t sz, const T& val)
      : StaticVector()
  {
    std::fill_n(mBuf.begin(), sz, val);
    mEnd += sz;
  }
  explicit StaticVector(size_t size)
      : StaticVector(size, T())
  {}
  StaticVector(const StaticVector& other)
      : mBuf(other.mBuf)
      , mEnd(mBuf.data() + other.size())
  {}
  StaticVector(StaticVector&& other)
      : mBuf(other.mBuf)
      , mEnd(mBuf.data() + other.size())
  {
    other.clear();
  }
  const StaticVector& operator=(const StaticVector& other)
  {
    mBuf = other.mBuf;
    mEnd = mBuf.data() + other.size();
    return *this;
  }
  const StaticVector& operator=(StaticVector&& other)
  {
    mBuf = other.mBuf;
    mEnd = mBuf.data() + other.size();
    other.clear();
    return *this;
  }
  void resize(size_t sz, const T& val)
  {
    if (sz < size()) {
      mEnd = mBuf.data() + sz;
    }
    else if (sz > size()) {
      sz -= size();
      std::fill_n(mEnd, sz, val);
      mEnd += sz;
    }
  }
  void     resize(size_t sz) { resize(sz, T()); }
  const T* begin() const { return mBuf.data(); }
  const T* end() const { return mEnd; }
  T*       begin() { return mBuf.data(); }
  T*       end() { return mEnd; }
  size_t   size() const { return size_t(std::distance(begin(), end())); }
  void     clear() { mEnd = mBuf.data(); }
  const T& operator[](size_t i) const { return mBuf[i]; }
  T&       operator[](size_t i) { return mBuf[i]; }
  bool     operator==(const StaticVector<T, N>& other) const
  {
    return size() == other.size() && std::equal(begin(), end(), other.begin());
  }
  void     push_back(const T& val) { *(mEnd++) = val; }
  const T& back() const { return *(mEnd - 1); }
  T&       back() { return *(mEnd - 1); }
  void     pop_back() { --mEnd; }
  bool     empty() const { return size() == 0; }
};

}  // namespace potato
