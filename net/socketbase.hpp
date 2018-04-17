#pragma once

#include "myspace/_/stdafx.hpp"
#include "myspace/net/socketopt.hpp"
MYSPACE_BEGIN

class Socketbase {
public:
  MYSPACE_EXCEPTION_DEFINE(SocketError, myspace::Exception)
  MYSPACE_EXCEPTION_DEFINE(ConnectError, SocketError)
  MYSPACE_EXCEPTION_DEFINE(Timeout, SocketError)

  ~Socketbase();

  int getFd() const;

  operator int() const;

  bool operator==(const Socketbase &s) const;

  const Addr &peer() const;

  const Addr &local() const;

  void bind(const Addr &addr);

  void bind(uint16_t port);

  void connect(const Addr &addr) noexcept(false);

  void
  connect(const Addr &addr,
          std::chrono::high_resolution_clock::duration timeout) noexcept(false);

  static void close(int sock);

protected:
  Socketbase(int family, int type, int protocal);

  int getSockError();

  void close();

protected:
  int sock_ = -1;
  int family_;
  int type_;
  int protocal_;
  Addr local_;
  Addr peer_;
};

inline Socketbase::Socketbase(int family, int type, int protocal)
    : family_(family), type_(type), protocal_(protocal) {}

inline Socketbase::~Socketbase() { close(); }

inline int Socketbase::getSockError() {
  int err = 0;
#if defined(MYSPACE_WINDOWS)
  int len = sizeof(err);
#else
  socklen_t len = sizeof(err);
#endif
  ::getsockopt(sock_, SOL_SOCKET, SO_ERROR, (char *)&err, &len);
  return err;
}

inline void Socketbase::connect(
    const Addr &addr,
    std::chrono::high_resolution_clock::duration timeout) noexcept(false) {
  peer_ = addr;
  this->close();
  sock_ = (int)::socket(family_, type_, protocal_);
  SocketOpt::setBlock(sock_, false);
  for (auto this_time = std::chrono::high_resolution_clock::now(),
            begin_time = this_time;
       this_time - begin_time <= timeout;
       this_time = std::chrono::high_resolution_clock::now()) {
    auto n =
        ::connect(sock_, (const sockaddr *)&peer_.addr(), sizeof(peer_.addr()));
    if (n == 0) {
      local_ = Addr::local(sock_);
      peer_ = Addr::peer(sock_);
      return;
    }
    auto e = Error::lastError();
    if (e == std::errc::already_connected)
      return;
    else if (e == std::errc::connection_already_in_progress ||
             e == std::errc::operation_in_progress ||
             e == std::errc::operation_would_block ||
             e == std::errc::interrupted) {
      auto sel = new_shared<Detector>();
      sel->add(this, DetectType::WRITE);
      sel->wait(timeout - (this_time - begin_time));
    } else {
      MYSPACE_THROW_EX(SocketError);
    }
  }
}

inline void Socketbase::connect(const Addr &addr) noexcept(false) {
  peer_ = addr;
  this->close();
  sock_ = (int)::socket(family_, type_, protocal_);
  SocketOpt::setBlock(sock_, true);
  for (;;) {
    auto n =
        ::connect(sock_, (const sockaddr *)&peer_.addr(), sizeof(peer_.addr()));
    if (n == 0) {
      local_ = Addr::local(sock_);
      peer_ = Addr::peer(sock_);
      return;
    }
    auto e = Error::lastError();
    if (e == std::errc::already_connected)
      return;
    else if (e == std::errc::connection_already_in_progress ||
             e == std::errc::operation_in_progress ||
             e == std::errc::operation_would_block ||
             e == std::errc::interrupted) {
      auto sel = new_shared<Detector>();
      sel->add(this, DetectType::WRITE);
      sel->wait();
    } else {
      MYSPACE_THROW_EX(SocketError);
    }
  }
}

inline void Socketbase::bind(const Addr &addr) {
  local_ = addr;
  MYSPACE_THROW_IF_EX(SocketError,
                      0 != ::bind(sock_, (const sockaddr *)&local_.addr(),
                                  sizeof(local_.addr())));
}

inline void Socketbase::bind(uint16_t port) {
  Addr addr{"0.0.0.0", port};
  this->bind(addr);
}

inline const Addr &Socketbase::peer() const { return peer_; }

inline const Addr &Socketbase::local() const { return local_; }

inline bool Socketbase::operator==(const Socketbase &s) const {
  return sock_ == s.sock_;
}

inline Socketbase::operator int() const { return sock_; }

inline int Socketbase::getFd() const { return sock_; }

inline void Socketbase::close() {
  Socketbase::close(sock_);
  sock_ = -1;
}

inline void Socketbase::close(int sock) {
#if defined(MYSPACE_WINDOWS)
  ::closesocket(sock);
#else
  ::close(sock);
#endif
}

MYSPACE_END
