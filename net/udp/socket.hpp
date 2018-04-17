
#pragma once

#include "myspace/_/stdafx.hpp"
#include "myspace/detector/detector.hpp"
#include "myspace/error/error.hpp"
#include "myspace/net/addr.hpp"
#include "myspace/net/socketbase.hpp"
#include "myspace/net/socketopt.hpp"
#include "myspace/strings/sstream.hpp"
MYSPACE_BEGIN
namespace udp {

class Socket : public myspace::Socketbase {
public:
  Socket();

  Socket(const Addr &addr) noexcept(false);

  Socket(const Addr &addr,
         std::chrono::high_resolution_clock::duration timeout) noexcept(false);

  ~Socket();

  size_t send(const std::string &data,
              std::chrono::high_resolution_clock::duration timeout);

  size_t send(const std::string &data);

  std::string recv(std::chrono::high_resolution_clock::duration timeout);

  std::string recv();

  size_t sendto(const Addr &, const std::string &data,
                std::chrono::high_resolution_clock::duration timeout);

  size_t sendto(const Addr &, const std::string &data);

  std::string recvfrom(Addr &,
                       std::chrono::high_resolution_clock::duration timeout);
};

inline Socket::Socket() : Socketbase(AF_INET, SOCK_DGRAM, 0) {}

inline Socket::Socket(const Addr &addr) noexcept(false)
    : Socketbase(AF_INET, SOCK_DGRAM, 0) {
  connect(addr);
}

inline Socket::Socket(
    const Addr &addr,
    std::chrono::high_resolution_clock::duration timeout) noexcept(false)
    : Socketbase(AF_INET, SOCK_DGRAM, 0) {
  connect(addr, timeout);
}

inline Socket::~Socket() { close(); }

inline size_t
Socket::send(const std::string &data,
             std::chrono::high_resolution_clock::duration timeout) {
  size_t sendn = 0;

  for (auto this_time = std::chrono::high_resolution_clock::now(),
            begin_time = this_time;
       sendn < data.size() && this_time - begin_time <= timeout;
       this_time = std::chrono::high_resolution_clock::now()) {
    auto n = ::send(sock_, data.c_str() + sendn, int(data.size() - sendn), 0);

    if (n > 0)
      sendn += n;

    else if (n == 0)
      break;

    else {
      auto e = Error::lastError();
      if (e == std::errc::operation_would_block ||
          e == std::errc::resource_unavailable_try_again ||
          e == std::errc::interrupted ||
          e == std::errc::operation_in_progress) {
        auto sel = new_shared<Detector>();
        sel->add(this, DetectType::WRITE);
        sel->wait(timeout - (this_time - begin_time));
        continue;
      }

      break;
    }
  }
  return sendn;
}

inline size_t Socket::send(const std::string &data) {
  size_t sendn = 0;

  SocketOpt::setBlock(sock_, true);

  while (sendn < data.size()) {
    auto n = ::send(sock_, data.c_str() + sendn, int(data.size() - sendn), 0);

    if (n > 0)
      sendn += n;

    else if (n == 0)
      break;

    else {
      auto e = Error::lastError();

      if (e == std::errc::operation_would_block ||
          e == std::errc::interrupted ||
          e == std::errc::operation_in_progress) {
        auto sel = new_shared<Detector>();
        sel->add(this, DetectType::WRITE);
        sel->wait();
        continue;
      }

      break;
    }
  }
  return sendn;
}

inline size_t Socket::sendto(const Addr &addr, const std::string &data) {
  size_t sendn = 0;

  SocketOpt::setBlock(sock_, true);

  while (sendn < data.size()) {
    auto n = ::sendto(sock_, data.c_str() + sendn, int(data.size() - sendn), 0,
                      (const sockaddr *)&addr.addr(), sizeof(addr.addr()));

    if (n > 0)
      sendn += n;

    else if (n == 0)
      break;

    else {
      auto e = Error::lastError();

      if (e == std::errc::operation_would_block ||
          e == std::errc::interrupted ||
          e == std::errc::operation_in_progress) {
        auto sel = new_shared<Detector>();
        sel->add(this, DetectType::WRITE);
        sel->wait();
        continue;
      }

      break;
    }
  }
  return sendn;
}

inline std::string
Socket::recv(std::chrono::high_resolution_clock::duration timeout) {
  std::string data;

  size_t buflen = 4096;

  auto buf = new_unique<char[]>(buflen);

  SocketOpt::setBlock(sock_, false);

  for (auto begin_time = std::chrono::high_resolution_clock::now(),
            this_time = begin_time;
       this_time - begin_time <= timeout;
       this_time = std::chrono::high_resolution_clock::now()) {
    auto n = ::recv(sock_, buf.get(), (int)buflen, 0);

    if (n > 0) {
      data.append(buf.get(), n);
      break;
    }

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

      break;
    }
  }
  return data;
}

inline std::string Socket::recv() {
  std::string data;

  size_t buflen = 4096;

  auto buf = new_unique<char[]>(buflen);

  SocketOpt::setBlock(sock_, true);

  auto n = ::recv(sock_, buf.get(), (int)buflen, 0);

  if (n > 0) {
    data.append(buf.get(), n);
  }
  return data;
} // namespace udp

inline std::string
Socket::recvfrom(Addr &dst,
                 std::chrono::high_resolution_clock::duration timeout) {
  std::string data;

  size_t buflen = 4096;

  auto buf = new_unique<char[]>(buflen);

  SocketOpt::setBlock(sock_, false);

  sockaddr_in addr = {0};
  addr.sin_family = AF_INET;
  MYSPACE_DEFER(dst = addr;);
  socklen_t addrlen = sizeof(addr);

  for (auto begin_time = std::chrono::high_resolution_clock::now(),
            this_time = begin_time;
       this_time - begin_time <= timeout;
       this_time = std::chrono::high_resolution_clock::now()) {

    auto n = ::recvfrom(sock_, buf.get(), (int)buflen, 0, (sockaddr *)&addr,
                        &addrlen);

    if (n > 0) {
      data.append(buf.get(), n);
      MYSPACE_DEV("buf[0] %s, buf[1]", (int)buf[0], (int)buf[1]);
      break;
    }

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

      break;
    }
  }

  return data;
}

} // namespace udp
MYSPACE_END
