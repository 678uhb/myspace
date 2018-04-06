

#pragma once

#include "myspace/_/include.hpp"

MYSPACE_BEGIN

class Path {
public:
  static string basename(const string &path);

  static string basenameNoext(const string &path);

  static deque<string> splitext(const string &path);

  static string dirname(const string &path);
};

inline string Path::basename(const string &path) {
  auto pos = path.find_last_of("\\/");

  if (pos != string::npos)
    return move(path.substr(pos + 1));

  return move(path);
}

inline string Path::basenameNoext(const string &path) {
  return move(splitext(basename(path))[0]);
}

inline deque<string> Path::splitext(const string &path) {
  auto pos = path.find_last_of('.');

  if (pos == path.npos) {
    return {path, ""};
  }

  return {path.substr(0, pos), path.substr(pos + 1)};
}

inline string Path::dirname(const string &path) {
  auto pos = path.find_last_of("/\\");

  if (pos == path.npos)
    return path;

  return move(path.substr(0, pos));
}
MYSPACE_END
