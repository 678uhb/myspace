
#pragma once

#include "myspace/_/include.hpp"
#include "myspace/memory/memory.hpp"
#include "myspace/path/path.hpp"

MYSPACE_BEGIN

class Process {
public:
  static string getMyFullName();

  static string getMyName();

  static string getMyNameNoExt();

  static string cwd(const string &path = "");
};

inline string Process::getMyFullName() {
  const size_t maxPath = 4096;

  auto name = new_unique<char[]>(maxPath);
  int n = 0;
#if defined(MYSPACE_WINDOWS)
  n = GetModuleFileName(NULL, name.get(), maxPath);
#elif defined(MYSPACE_LINUX)
  n = readlink("/proc/self/exe", name.get(), maxPath);
#endif
  if (n <= 0)
    return "";
  return string(name.get(), n);
}

inline string Process::getMyName() {
  return Path::basename(getMyFullName());
}

inline string Process::getMyNameNoExt() {
  return Path::basenameNoext(getMyFullName());
}

inline string Process::cwd(const string &path) {
  if (path.empty()) {
    const size_t maxPath = 4096;

    auto name = new_unique<char[]>(maxPath);

#if defined(MYSPACE_WINDOWS)

    auto n = GetCurrentDirectory(maxPath, name.get());

    if (n <= 0)
      return "";

    return string(name.get(), n);
#else
    auto n = readlink("/proc/self/cwd", name.get(), maxPath);

    if (n <= 0)
      return "";

    return string(name.get(), n);
#endif
  }
#if defined(MYSPACE_WINDOWS)
  SetCurrentDirectory(path.c_str());
#else
  chdir(path.c_str());
#endif
  return cwd();
}

MYSPACE_END
