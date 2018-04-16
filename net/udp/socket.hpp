
#pragma once

#include "myspace/_/stdafx.hpp"
#include "myspace/detector/detector.hpp"
#include "myspace/error/error.hpp"
#include "myspace/net/addr.hpp"
#include "myspace/net/socketopt.hpp"
#include "myspace/strings/sstream.hpp"

MYSPACE_BEGIN
namespace udp {

class Socket {
public:
  Socket();

  Socket(const Addr &addr,
         std::chrono::high_resolution_clock::duration timeout =
             std::chrono::hours(999));

  ~Socket();

  Socket &connect(const Addr &addr,
                  std::chrono::high_resolution_clock::duration timeout =
                      std::chrono::hours(999));

  Socket &bind(const Addr &addr);

  Socket &bind(uint16_t port);

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

  Socket &setBlock();

  Socket &setNonBlock();

  bool isBlocked() const;

  //operator bool();

  int getFd() const;

  //bool isConnected();

  void close();

  static void close(int sock);
  bool operator==(const Socket &s) const;

  operator int() const;

  const Addr &getPeer() const;

  const Addr &getLocal() const;

protected:
  Socket &setBlock(bool f);

  int getSockError();

  bool is_blocked_ = true;
  int sock_ = -1;
  Addr local_;
  Addr peer_;
  int domain_ = AF_INET;
  int type_ = SOCK_DGRAM;
  int protocal_ = 0;
};

inline Socket::Socket() { sock_ = socket(domain_, type_, protocal_); }

inline Socket::Socket(const Addr &addr,
                      std::chrono::high_resolution_clock::duration timeout) {
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
      auto e = Error::lastNetError();

      if (e == Error::WOULD_BLOCK || e == Error::INTERRUPTED ||
          e == Error::IN_PROGRESS) {
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

  setBlock();

  while (sendn < data.size()) {
    auto n = ::send(sock_, data.c_str() + sendn, int(data.size() - sendn), 0);

    if (n > 0)
      sendn += n;

    else if (n == 0)
      break;

    else {
      auto e = Error::lastNetError();

      if (e == Error::WOULD_BLOCK || e == Error::INTERRUPTED ||
          e == Error::IN_PROGRESS) {
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

  setBlock();

  while (sendn < data.size()) {
    auto n = ::sendto(sock_, data.c_str() + sendn, int(data.size() - sendn), 0,
                      (const sockaddr *)&addr.addr(), sizeof(addr.addr()));

    if (n > 0)
      sendn += n;

    else if (n == 0)
      break;

    else {
      auto e = Error::lastNetError();

      if (e == Error::WOULD_BLOCK || e == Error::INTERRUPTED ||
          e == Error::IN_PROGRESS) {
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

inline Socket &Socket::bind(const Addr &addr) {
  local_ = addr;
  ::bind(sock_, (const sockaddr *)&local_.addr(), sizeof(local_.addr()));
  return *this;
}

inline Socket &Socket::bind(uint16_t port) {
  Addr addr{"0.0.0.0", port};
  this->bind(addr);
  return *this;
}

inline std::string
Socket::recv(std::chrono::high_resolution_clock::duration timeout) {
  std::string data;

  size_t buflen = 4096;

  auto buf = new_unique<char[]>(buflen);

  setNonBlock();

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
      auto e = Error::lastNetError();

      if (e == Error::WOULD_BLOCK || e == Error::INTERRUPTED ||
          e == Error::IN_PROGRESS) {
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

  setBlock();

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

  setNonBlock();

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
      auto e = Error::lastNetError();

      if (e == Error::WOULD_BLOCK || e == Error::INTERRUPTED ||
          e == Error::IN_PROGRESS) {
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

inline Socket &Socket::setBlock() { return setBlock(true); }

inline Socket &Socket::setNonBlock() { return setBlock(false); }

inline bool Socket::isBlocked() const { return is_blocked_; }

//inline Socket::operator bool() { return isConnected(); }

inline int Socket::getFd() const { return sock_; }

//inline bool Socket::isConnected() {
//  auto isblocked = is_blocked_;
//  MYSPACE_DEFER(setBlock(isblocked));
//again:
//  char c;
//  setNonBlock();
//  auto n = ::recv(sock_, &c, 1, MSG_PEEK);
//  if (n == 0)
//    return false;
//  if (n > 0)
//    return true;
//  auto e = Error::lastNetError();
//
//  switch (e) {
//  case Error::WOULD_BLOCK:
//  case Error::IN_PROGRESS:
//    return true;
//  case Error::INTERRUPTED:
//    goto again;
//  default:
//    return false;
//  }
//}

inline void Socket::close() {
  if (sock_ >= 0) {
    setBlock(true);
    Socket::close(sock_);
    sock_ = -1;
  }
}

inline void Socket::close(int sock) {
#if defined(MYSPACE_WINDOWS)
  ::closesocket(sock);
#else
  ::close(sock);
#endif
}

inline bool Socket::operator==(const Socket &s) const {
  return sock_ == s.sock_;
}

inline Socket::operator int() const { return sock_; }

inline Socket &
Socket::connect(const Addr &addr,
                std::chrono::high_resolution_clock::duration timeout) {
  peer_ = addr;

  this->close();

  sock_ = (int)::socket(domain_, type_, protocal_);

  setNonBlock();

  for (auto this_time = std::chrono::high_resolution_clock::now(),
            begin_time = this_time;
       this_time - begin_time <= timeout;
       this_time = std::chrono::high_resolution_clock::now()) {

    auto n =
        ::connect(sock_, (const sockaddr *)&peer_.addr(), sizeof(peer_.addr()));

    if (n == 0) {
      local_ = Addr::getLocal(sock_);
      peer_ = Addr::getPeer(sock_);
      return *this;
    }

    auto e = Error::lastNetError();

    switch (e) {
    case Error::ISCONN:
      return *this;
    case Error::ALREADY:
    case Error::IN_PROGRESS:
    case Error::WOULD_BLOCK:
    case Error::INTERRUPTED: {
   /*   if (isConnected())
        return *this;*/

      auto sel = new_shared<Detector>();

      sel->add(this);

      sel->wait(timeout - (this_time - begin_time));
      break;
    }
    default: {
      this->close();
      sock_ = (int)::socket(domain_, type_, protocal_);
      setNonBlock();
      std::this_thread::sleep_for(
          std::min(std::chrono::milliseconds(100),
                   std::chrono::duration_cast<std::chrono::milliseconds>(
                       timeout - (this_time - begin_time))));
      break;
    }
    }
  }
  return *this;
}

inline Socket &Socket::setBlock(bool f) {
  SocketOpt::setBlock(sock_, f);
  is_blocked_ = f;
  return *this;
}

inline int Socket::getSockError() {
  int err = 0;
#if defined(MYSPACE_WINDOWS)
  int len = sizeof(err);
#else
  socklen_t len = sizeof(err);
#endif
  ::getsockopt(sock_, SOL_SOCKET, SO_ERROR, (char *)&err, &len);
  return err;
}

inline const Addr &Socket::getPeer() const { return peer_; }

inline const Addr &Socket::getLocal() const { return local_; }

} // namespace udp
MYSPACE_END
