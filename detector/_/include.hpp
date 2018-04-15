
#pragma once

#include "myspace/_/stdafx.hpp"

MYSPACE_BEGIN

enum DetectType {
#if defined(MYSPACE_LINUX)
  READ = EPOLLIN,
  WRITE = EPOLLOUT,
  EXCEP = EPOLLERR,
  READ_WRITE = READ | WRITE,
#else
  READ = 1,
  WRITE = (1 << 1),
  EXCEP = (1 << 2),
  READ_WRITE = READ | WRITE,
#endif
};

namespace DetectorImpl {
struct Candidate {
  Any _x;

  DetectType _ev;
};
} // namespace DetectorImpl

MYSPACE_END
