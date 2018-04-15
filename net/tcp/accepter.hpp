
#pragma once

#include "myspace/_/stdafx.hpp"
#include "myspace/exception/exception.hpp"
#include "myspace/net/tcp/socket.hpp"
MYSPACE_BEGIN

namespace tcp {

class Accepter {
public:
  MYSPACE_EXCEPTION_DEFINE(Exception, myspace::Exception)
public:
  Accepter();

  Accepter(uint16_t port) throw(Accepter::Exception);

  ~Accepter();

  std::shared_ptr<tcp::Socket>
  accept(std::chrono::high_resolution_clock::duration timeout);

  std::shared_ptr<tcp::Socket> accept();

  operator int() const;

private:
  int sock_ = tcp::Socket();
};

inline Accepter::Accepter() {}

inline Accepter::Accepter(uint16_t port) throw(Accepter::Exception) {
  auto sock = (int)::socket(AF_INET, SOCK_STREAM, 0);
  Defer xs([&sock]() { Socket::close(sock); });

  SocketOpt::reuseAddr(sock, true);

  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = INADDR_ANY;

  MYSPACE_THROW_IF_EX(Accepter::Exception,
                      0 != ::bind(sock, (sockaddr *)&addr, sizeof(addr)));
  MYSPACE_THROW_IF_EX(Accepter::Exception, 0 != ::listen(sock, 1024));

  xs.dismiss();
  sock_ = sock;
}

inline std::shared_ptr<tcp::Socket>
Accepter::accept(std::chrono::high_resolution_clock::duration timeout) {
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
    }

    auto sel = new_shared<Detector>();

    sel->add(this);

    sel->wait(timeout - (this_time - begin_time));
  }
  return nullptr;
}

inline std::shared_ptr<tcp::Socket> Accepter::accept() {
  for (;;) {
    sockaddr_in addr;

    socklen_t addrlen = sizeof(addr);

    auto n = (int)::accept(sock_, (sockaddr *)&addr, &addrlen);

    if (n >= 0) {
      auto sock = new_shared<tcp::Socket>(n);
      return sock;
    }

    auto sel = new_shared<Detector>();

    sel->add(this);

    sel->wait();
  }
  return nullptr;
}

inline Accepter::~Accepter() { Socket::close(sock_); }

inline Accepter::operator int() const { return sock_; }
} // namespace tcp

MYSPACE_END
