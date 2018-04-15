
#pragma once

#include "myspace/_/stdafx.hpp"

MYSPACE_BEGIN

class SocketOpt {
public:
  static void setBlock(int fd, bool f);

  static void reuseAddr(int sock, bool f);
};

inline void SocketOpt::setBlock(int fd, bool f) {
#if defined(MYSPACE_LINUX)
  auto flags = ::fcntl(fd, F_GETFL, 0);
  if (!f)
    ::fcntl(fd, F_SETFL, flags | O_NONBLOCK);
  else
    ::fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
#endif
#if defined(MYSPACE_WINDOWS)
  int mode = (f ? 0 : 1);
  ::ioctlsocket(fd, FIONBIO, (u_long FAR *)&mode);
#endif
}

inline void SocketOpt::reuseAddr(int sock, bool f) {
  int on = (f ? 1 : 0);
  ::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
}

MYSPACE_END
