
#pragma once

#include "myspace/_/stdafx.hpp"
#include "myspace/defer/defer.hpp"

MYSPACE_BEGIN

class Error {
public:
  static int lastError();

  static int lastNetError();

  static std::string strerror(int ec);

  enum {
#if defined(MYSPACE_WINDOWS)
    WOULD_BLOCK = WSAEWOULDBLOCK,
    // err_again = EAGAIN,
    IN_PROGRESS = WSAEINPROGRESS,
    INTERRUPTED = WSAEINTR,
    ALREADY = WSAEALREADY,
    ISCONN = WSAEISCONN,
    MSGSIZE = WSAEMSGSIZE,
#else
    WOULD_BLOCK = EWOULDBLOCK,
    // err_again	= EAGAIN,
    IN_PROGRESS = EINPROGRESS,
    INTERRUPTED = EINTR,
    ALREADY = EALREADY,
    ISCONN = EISCONN,
    MSGSIZE = EMSGSIZE,
#endif
  };
};
inline int Error::lastError() {
#if defined(MYSPACE_WINDOWS)
  return ::GetLastError();
#else
  return errno;
#endif
}
inline int Error::lastNetError() {
#if defined(MYSPACE_WINDOWS)
  return ::WSAGetLastError();
#else
  return errno;
#endif
}
inline std::string Error::strerror(int ec) {
#if defined(MYSPACE_WINDOWS)
  LPVOID buf = nullptr;
  MYSPACE_DEFER(::LocalFree(buf));
  ::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                  nullptr, ec, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPTSTR)&buf, 0, nullptr);
  return std::string((char *)buf);
#else
  auto buf = new_unique<char[]>(1024);
  return std::string(::strerror_r(ec, buf.get(), 1024));
#endif
}

MYSPACE_END
