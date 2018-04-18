
#pragma once

#include "myspace/_/stdafx.hpp"
#include "myspace/detector/detector.hpp"
#include "myspace/error/error.hpp"
#include "myspace/exception/exception.hpp"
#include "myspace/net/addr.hpp"
#include "myspace/net/socketbase.hpp"
#include "myspace/net/socketopt.hpp"
#include "myspace/strings/sstream.hpp"
MYSPACE_BEGIN
namespace tcp {

class Socket : public myspace::Socketbase {
public:
  Socket();

  Socket(int sock);

  Socket(const Addr &addr) noexcept(false);

  Socket(const Addr &addr,
         std::chrono::high_resolution_clock::duration timeout) noexcept(false);

  ~Socket();

  size_t send(const std::string &data) noexcept(false);

  size_t
  send(const std::string &data,
       std::chrono::high_resolution_clock::duration timeout) noexcept(false);

  std::string
  recv(std::chrono::high_resolution_clock::duration timeout) noexcept(false);

  std::string
  recv(size_t len,
       std::chrono::high_resolution_clock::duration timeout) noexcept(false);

  std::string recv(size_t len) noexcept(false);

  std::string recvUntil(const std::string &delm) noexcept(false);

  std::string recvUntil(
      const std::string &delm,
      std::chrono::high_resolution_clock::duration timeout) noexcept(false);
};

inline Socket::Socket() : Socketbase(AF_INET, SOCK_STREAM, 0) {}

inline Socket::Socket(int sock) : Socketbase(AF_INET, SOCK_STREAM, 0) {
  sock_ = sock;
  local_ = Addr::local(sock);
  peer_ = Addr::peer(sock);
}

inline Socket::Socket(const Addr &addr) noexcept(false)
    : Socketbase(AF_INET, SOCK_STREAM, 0) {
  connect(addr);
}

inline Socket::Socket(
    const Addr &addr,
    std::chrono::high_resolution_clock::duration timeout) noexcept(false)
    : Socketbase(AF_INET, SOCK_STREAM, 0) {
  connect(addr, timeout);
}

inline Socket::~Socket() { close(); }

inline size_t Socket::send(
    const std::string &data,
    std::chrono::high_resolution_clock::duration timeout) noexcept(false) {

  if (data.empty())
    return 0;

  SocketOpt::setBlock(sock_, false);

  size_t sendn = 0;

  for (auto this_time = std::chrono::high_resolution_clock::now(),
            begin_time = this_time;
       sendn < data.size() && this_time - begin_time <= timeout;
       this_time = std::chrono::high_resolution_clock::now()) {

    auto n = ::send(sock_, data.c_str() + sendn, int(data.size() - sendn), 0);

    if (n > 0) {
      sendn += n;
    } else if (n == 0) {
      break;
    } else {
      auto e = Error::lastError();
      if (e == std::errc::operation_would_block ||
          e == std::errc::interrupted ||
          e == std::errc::operation_in_progress) {
        auto sel = new_shared<Detector>();
        sel->add(this, DetectType::WRITE);
        sel->wait(timeout - (this_time - begin_time));
        continue;
      }
      MYSPACE_THROW_IF_EX(SocketError, sendn == 0);
      break;
    }
  }
  return sendn;
}

inline size_t Socket::send(const std::string &data) noexcept(false) {

  if (data.empty())
    return 0;

  size_t sendn = 0;

  SocketOpt::setBlock(sock_, true);

  while (sendn < data.size()) {
    auto n = ::send(sock_, data.c_str() + sendn, int(data.size() - sendn), 0);
    if (n > 0) {
      sendn += n;
    } else if (n == 0) {
      break;
    } else {
      auto e = Error::lastError();
      if (e == std::errc::operation_would_block ||
          e == std::errc::interrupted ||
          e == std::errc::operation_in_progress) {
        auto sel = new_shared<Detector>();
        sel->add(this, DetectType::WRITE);
        sel->wait();
        continue;
      }
      MYSPACE_THROW_IF_EX(SocketError, sendn == 0);
      break;
    }
  }
  return sendn;
}

inline std::string Socket::recv(
    std::chrono::high_resolution_clock::duration timeout) noexcept(false) {
  std::string data;

  size_t buflen = 4096;

  auto buf = new_unique<char[]>(buflen);

  SocketOpt::setBlock(sock_, false);

  for (auto begin_time = std::chrono::high_resolution_clock::now(),
            this_time = begin_time;
       this_time - begin_time <= timeout;
       this_time = std::chrono::high_resolution_clock::now()) {
    auto n = ::recv(sock_, buf.get(), (int)buflen, 0);

    if (n > 0)
      data.append(buf.get(), n);

    else if (n == 0)
      break;

    else {
      auto e = Error::lastError();

      if (e == std::errc::operation_would_block ||
          e == std::errc::interrupted ||
          e == std::errc::operation_in_progress) {
        auto sel = new_shared<Detector>();
        sel->add(this);
        sel->wait(timeout - (this_time - begin_time));
        continue;
      }
      MYSPACE_THROW_IF_EX(SocketError, data.empty());
      break;
    }
  }
  return data;
}

inline std::string Socket::recv(
    size_t len,
    std::chrono::high_resolution_clock::duration timeout) noexcept(false) {
  if (len == 0)
    return "";

  size_t recvn = 0;

  SocketOpt::setBlock(sock_, false);

  auto buf = new_unique<char[]>(len);

  for (auto this_time = std::chrono::high_resolution_clock::now(),
            begin_time = this_time;
       recvn < len && this_time - begin_time <= timeout;
       this_time = std::chrono::high_resolution_clock::now()) {
    auto n = ::recv(sock_, buf.get() + recvn, int(len - recvn), 0);

    if (n > 0) {
      recvn += n;
    }

    else if (n == 0)
      break;

    else {
      auto e = Error::lastError();
      MYSPACE_DEV(" errno = ", e);
      if (e == std::errc::operation_would_block ||
          e == std::errc::interrupted ||
          e == std::errc::operation_in_progress) {
        auto sel = new_shared<Detector>();
        sel->add(this);
        sel->wait(timeout - (this_time - begin_time));
        continue;
      }
      MYSPACE_DEV("throw here");
      MYSPACE_THROW_IF_EX(SocketError, recvn == 0);
      break;
    }
  }
  return std::string(buf.get(), recvn);
}

inline std::string Socket::recv(size_t len) noexcept(false) {
  if (len == 0)
    return "";

  size_t recvn = 0;

  auto buf = new_unique<char[]>(len);

  SocketOpt::setBlock(sock_, true);

  while (recvn < len) {
    auto n = ::recv(sock_, buf.get() + recvn, int(len - recvn), 0);

    if (n > 0) {
      recvn += n;
    }

    else if (n == 0)
      break;

    else {
      auto e = Error::lastError();
      if (e == std::errc::operation_would_block ||
          e == std::errc::interrupted ||
          e == std::errc::operation_in_progress) {
        continue;
      }
      MYSPACE_THROW_IF_EX(SocketError, recvn == 0);
      break;
    }
  }
  return std::string(buf.get(), recvn);
}

inline std::string Socket::recvUntil(const std::string &delm) noexcept(false) {
  if (delm.empty())
    return "";

  SocketOpt::setBlock(sock_, true);

  size_t recvn = delm.size();

  std::string ret;

  auto buf = new_unique<char[]>(recvn);

  while (recvn) {
    auto n = ::recv(sock_, buf.get(), int(recvn), 0);
    if (n == 0) {
      break;
    }
    if (n < 0) {
      auto e = Error::lastError();

      if (e == std::errc::operation_would_block ||
          e == std::errc::interrupted ||
          e == std::errc::operation_in_progress) {
        auto sel = new_shared<Detector>();
        sel->add(this);
        sel->wait();
        continue;
      }
      break;
    }

    ret.append(buf.get(), n);
    recvn = Strings::endWithLess(ret, delm);
    if (recvn == 0)
      break;
  }
  MYSPACE_THROW_IF_EX(SocketError, ret.empty());
  return ret;
}

inline std::string Socket::recvUntil(
    const std::string &delm,
    std::chrono::high_resolution_clock::duration timeout) noexcept(false) {
  if (delm.empty())
    return recv(timeout);

  SocketOpt::setBlock(sock_, false);

  size_t recvn = delm.size();

  std::string ret;

  auto buf = new_unique<char[]>(recvn);

  for (auto this_time = std::chrono::system_clock::now(),
            begin_time = this_time;
       (ret.size() < delm.size() ||
        ret.rfind(delm, ret.size() - delm.size()) == ret.npos) &&
       this_time - begin_time <= timeout;
       this_time = std::chrono::system_clock::now()) {
    auto n = ::recv(sock_, buf.get(), int(recvn), 0);

    if (n == 0)
      break;

    if (n < 0) {
      auto e = Error::lastError();

      if (e == std::errc::operation_would_block ||
          e == std::errc::interrupted ||
          e == std::errc::operation_in_progress) {
        auto sel = new_shared<Detector>();
        sel->add(this);
        sel->wait(timeout - (this_time - begin_time));
        continue;
      }
      MYSPACE_THROW_IF_EX(SocketError, ret.empty());
      break;
    }

    ret.append(buf.get(), n);
    recvn = Strings::endWithLess(ret, delm);
    if (recvn == 0)
      break;
  }
  return ret;
}

} // namespace tcp
MYSPACE_END
