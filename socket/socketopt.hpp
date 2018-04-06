
#pragma once

#include "myspace/_/include.hpp"

MYSPACE_BEGIN

class SocketOpt {
public:
  static void setBlock(int fd, bool f);

  static void reuseAddr(int sock, bool f);
};

void SocketOpt::setBlock(int fd, bool f) {
#ifdef MYSPACE_LINUX
  auto flags = ::fcntl(fd, F_GETFL, 0);
  if (!f)
    ::fcntl(fd, F_SETFL, flags | O_NONBLOCK);
  else
    ::fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
#endif
#ifdef MYSPACE_WINDOWS
  int iMode = (f ? 0 : 1);
  ::ioctlsocket(fd, FIONBIO, (u_long FAR *)&iMode);
#endif
}

void SocketOpt::reuseAddr(int sock, bool f) {
  int on = (f ? 1 : 0);
  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
}

MYSPACE_END
