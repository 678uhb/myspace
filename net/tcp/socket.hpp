
#pragma once

#include "myspace/_/stdafx.hpp"
#include "myspace/detector/detector.hpp"
#include "myspace/error/error.hpp"
#include "myspace/exception/exception.hpp"
#include "myspace/net/addr.hpp"
#include "myspace/net/socketopt.hpp"
#include "myspace/strings/sstream.hpp"
MYSPACE_BEGIN
namespace tcp {

// MYSPACE_EXCEPTION_DEFINE(SocketError, Exception)

class Socket {
public:
  Socket();

  Socket(int sock);

  Socket(const Addr &addr,
         std::chrono::high_resolution_clock::duration timeout =
             std::chrono::hours(999));

  ~Socket();

  Socket &connect(const Addr &addr,
                  std::chrono::high_resolution_clock::duration timeout);

  size_t send(const std::string &data,
              std::chrono::high_resolution_clock::duration timeout);

  size_t send(const std::string &data);

  std::string recv(std::chrono::high_resolution_clock::duration timeout);

  std::string recv(size_t len,
                   std::chrono::high_resolution_clock::duration timeout);

  std::string recv(size_t len);

  std::string recv_until(const std::string &delm,
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
  int type_ = SOCK_STREAM;
  int protocal_ = 0;
};

inline Socket::Socket() {}

inline Socket::Socket(int sock) {
  sock_ = sock;
  peer_ = Addr::getPeer(sock);
  local_ = Addr::getLocal(sock);
}

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

    if (n > 0)
      data.append(buf.get(), n);

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

inline std::string
Socket::recv(size_t len, std::chrono::high_resolution_clock::duration timeout) {
  if (len == 0)
    return "";

  size_t recvn = 0;

  setNonBlock();

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
      auto e = Error::lastNetError();
      MYSPACE_DEV(" errno = ", e);
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
  return std::string(buf.get(), recvn);
}

inline std::string Socket::recv(size_t len) {
  if (len == 0)
    return "";

  size_t recvn = 0;

  auto buf = new_unique<char[]>(len);

  setBlock();

  while (recvn < len) {
    auto n = ::recv(sock_, buf.get() + recvn, int(len - recvn), 0);

    if (n > 0) {
      recvn += n;
    }

    else if (n == 0)
      break;

    else {
      auto e = Error::lastNetError();
      if (e == Error::WOULD_BLOCK || e == Error::INTERRUPTED ||
          e == Error::IN_PROGRESS) {
        continue;
      }
      break;
    }
  }
  return std::string(buf.get(), recvn);
}

inline std::string
Socket::recv_until(const std::string &delm,
                   std::chrono::high_resolution_clock::duration timeout) {
  if (delm.empty())
    return recv(timeout);

  setNonBlock();

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

    ret.append(buf.get(), n);
    recvn = 0;
    for (size_t maxmatchn = std::min(delm.size(), ret.size()); maxmatchn;
         --maxmatchn) {
      if (ret.compare(ret.size() - maxmatchn, maxmatchn, delm, 0, maxmatchn) ==
          0) {
        recvn = delm.size() - maxmatchn;
        break;
      }
    }
    if (!recvn)
      recvn = delm.size();
  }
  return ret;
}

inline Socket &Socket::setBlock() { return setBlock(true); }

inline Socket &Socket::setNonBlock() { return setBlock(false); }

inline bool Socket::isBlocked() const { return is_blocked_; }

//inline Socket::operator bool() { return isConnected(); }

inline int Socket::getFd() const { return sock_; }
//
//inline bool Socket::isConnected() {
//  auto isblocked = is_blocked_;
//  MYSPACE_DEFER(setBlock(isblocked));
//again:
//  char c;
//  setNonBlock();
//  MYSPACE_DEV("is connected ?");
//  auto n = ::recv(sock_, &c, 1, MSG_PEEK);
//  MYSPACE_DEV("is connected ?? n = ", n);
//  if (n == 0) {
//    MYSPACE_DEV("is connected ??, broken ! n = ", n);
//    return false;
//  }
//
//  if (n > 0) {
//    MYSPACE_DEV("is connected ?? yes, return > 0 , n = ", n);
//    return true;
//  }
//  auto e = Error::lastNetError();
//
//  switch (e) {
//  case Error::WOULD_BLOCK:
//  case Error::IN_PROGRESS:
//    MYSPACE_DEV("is connected ?? yes ");
//    return true;
//  case Error::INTERRUPTED:
//    MYSPACE_DEV("is connected ?? sorry, again! ");
//    goto again;
//  default:
//    MYSPACE_DEV("is connected ?? no! err = ", e);
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

  this->close();

  sock_ = (int)::socket(domain_, type_, protocal_);

  setNonBlock();

  peer_ = addr;

  for (auto this_time = std::chrono::high_resolution_clock::now(),
            begin_time = this_time;
       this_time - begin_time <= timeout;
       this_time = std::chrono::high_resolution_clock::now()) {

    MYSPACE_DEV(" before connect ");
    auto n = ::connect(sock_, (sockaddr *)&peer_.addr(), sizeof(peer_.addr()));
    MYSPACE_DEV(" after connect ");

    if (n == 0) {
      local_ = Addr::getLocal(sock_);
      peer_ = Addr::getPeer(sock_);
      return *this;
    }

    auto e = Error::lastNetError();

    MYSPACE_DEV("errno =", e);
    switch (e) {
    case Error::ISCONN:
      return *this;
    case Error::WOULD_BLOCK:
    case Error::ALREADY:
    case Error::INTERRUPTED:
    case Error::IN_PROGRESS: {
      /*if (isConnected())
        return *this;*/

      auto sel = new_shared<Detector>();

      sel->add(this, DetectType::WRITE);
      MYSPACE_DEV("before wait");
      sel->wait(timeout - (this_time - begin_time));
      MYSPACE_DEV("after wait");
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
} // namespace tcp

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

} // namespace tcp
MYSPACE_END
