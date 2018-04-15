
#pragma once

#include "myspace/_/stdafx.hpp"

MYSPACE_BEGIN

class OS {
public:
  static size_t filesize(const std::string &path);
};

inline size_t OS::filesize(const std::string &path) {
  std::ifstream ifs(path, std::ios::in | std::ios::binary);
  if (!ifs.is_open())
    return 0;
  ifs.seekg(0, std::ios::end);
  return ifs.tellg();
}

MYSPACE_END
