
#pragma once

#include "myspace/_/stdafx.hpp"
#include "myspace/exception/exception.hpp"
#include "myspace/net/tcp/socket.hpp"
MYSPACE_BEGIN

namespace tcp {

class Acceptor {
public:
  MYSPACE_EXCEPTION_DEFINE(AcceptorError, myspace::Exception)

public:
  Acceptor();

  Acceptor(uint16_t port) noexcept(false);

  ~Acceptor();

  std::shared_ptr<tcp::Socket>
  accept(std::chrono::high_resolution_clock::duration timeout) noexcept(false);

  std::shared_ptr<tcp::Socket> accept() noexcept(false);

  operator int() const;

private:
  int sock_ = -1;
};

inline Acceptor::Acceptor() {}

inline Acceptor::Acceptor(uint16_t port) noexcept(false) {
  auto sock = (int)::socket(AF_INET, SOCK_STREAM, 0);
  Defer xs([&sock]() { Socket::close(sock); });

  SocketOpt::reuseAddr(sock, true);

  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = INADDR_ANY;

  MYSPACE_THROW_IF_EX(AcceptorError,
                      0 != ::bind(sock, (sockaddr *)&addr, sizeof(addr)));
  MYSPACE_THROW_IF_EX(AcceptorError, 0 != ::listen(sock, 1024));

  xs.dismiss();
  sock_ = sock;
}

inline std::shared_ptr<tcp::Socket> Acceptor::accept(
    std::chrono::high_resolution_clock::duration timeout) noexcept(false) {

  SocketOpt::setBlock(sock_, false);

  for (auto this_time = std::chrono::high_resolution_clock::now(),
            begin_time = this_time;
       this_time - begin_time <= timeout;
       this_time = std::chrono::high_resolution_clock::now()) {
    sockaddr_in addr;

    socklen_t addrlen = sizeof(addr);

    auto n = (int)::accept(sock_, (sockaddr *)&addr, &addrlen);

    if (n >= 0) {
      auto sock = new_shared<tcp::Socket>(n);
      return sock;
    } else {
      auto e = Error::lastError();
      if (e == std::errc::not_a_socket || e == std::errc::invalid_argument) {
        MYSPACE_THROW_EX(AcceptorError);
      }
      auto sel = new_shared<Detector>();
      sel->add(this);
      sel->wait(timeout - (this_time - begin_time));
    }
  }
  return nullptr;
}

inline std::shared_ptr<tcp::Socket> Acceptor::accept() noexcept(false) {

  SocketOpt::setBlock(sock_, true);

  for (;;) {
    sockaddr_in addr;

    socklen_t addrlen = sizeof(addr);

    auto n = (int)::accept(sock_, (sockaddr *)&addr, &addrlen);

    if (n >= 0) {
      auto sock = new_shared<tcp::Socket>(n);
      return sock;
    }
    auto e = Error::lastError();
    if (e == std::errc::not_a_socket || e == std::errc::invalid_argument) {
      MYSPACE_THROW_EX(AcceptorError);
    }
    auto sel = new_shared<Detector>();
    sel->add(this);
    sel->wait();
  }
  return nullptr;
}

inline Acceptor::~Acceptor() { Socket::close(sock_); }

inline Acceptor::operator int() const { return sock_; }
} // namespace tcp

MYSPACE_END
