
#pragma once

#include "myspace/_/stdafx.hpp"
#include "myspace/memory/memory.hpp"
#include "myspace/path/path.hpp"

MYSPACE_BEGIN

class Process {
public:
  static std::string getMyFullName();

  static std::string getMyName();

  static std::string getMyNameNoExt();

  static std::string cwd(const std::string &path = "");
};

namespace processimpl {
inline std::string pwd() {
  int n = 0;
  static constexpr size_t max_path = 4096;
  auto name = newUnique<char[]>(max_path);
#if defined(MYSPACE_WINDOWS)
  n = ::GetCurrentDirectory(max_path, name.get());
#else
  n = ::readlink("/proc/self/cwd", name.get(), max_path);
#endif
  if (n <= 0)
    return "";
  return std::string(name.get(), n);
}
} // namespace processimpl

inline std::string Process::getMyFullName() {
  static constexpr size_t max_path = 4096;
  auto name = newUnique<char[]>(max_path);
  int n = 0;
#if defined(MYSPACE_WINDOWS)
  n = ::GetModuleFileName(nullptr, name.get(), max_path);
#elif defined(MYSPACE_LINUX)
  n = ::readlink("/proc/self/exe", name.get(), max_path);
#endif
  if (n <= 0)
    return "";
  return std::string(name.get(), n);
}

inline std::string Process::getMyName() {
  return Path::basename(getMyFullName());
}

inline std::string Process::getMyNameNoExt() {
  return Path::basenameNoext(getMyFullName());
}

inline std::string Process::cwd(const std::string &path) {
  if (path.empty()) {
    return processimpl::pwd();
  }
#if defined(MYSPACE_WINDOWS)
  SetCurrentDirectory(path.c_str());
#else
  chdir(path.c_str());
#endif
  return cwd();
}

MYSPACE_END
