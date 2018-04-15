

#pragma once

#include "myspace/_/stdafx.hpp"

MYSPACE_BEGIN

class Path {
public:
  static std::string basename(const std::string &path);

  static std::string basenameNoext(const std::string &path);

  static std::deque<std::string> splitext(const std::string &path);

  static std::string dirname(const std::string &path);
};

inline std::string Path::basename(const std::string &path) {
  auto pos = path.find_last_of("\\/");

  if (pos != std::string::npos)
    return path.substr(pos + 1);

  return path;
}

inline std::string Path::basenameNoext(const std::string &path) {
  return std::move(splitext(basename(path))[0]);
}

inline std::deque<std::string> Path::splitext(const std::string &path) {
  auto pos = path.find_last_of('.');

  if (pos == path.npos) {
    return {path, ""};
  }

  return {path.substr(0, pos), path.substr(pos + 1)};
}

inline std::string Path::dirname(const std::string &path) {
  auto pos = path.find_last_of("/\\");

  if (pos == path.npos)
    return path;

  return path.substr(0, pos);
}
MYSPACE_END
