
#pragma once

#include "myspace/_/include.hpp"
#include "myspace/detector/detector.hpp"
#include "myspace/error/error.hpp"
#include "myspace/socket/socketopt.hpp"

MYSPACE_BEGIN

class Socket {
public:
  Socket();

  Socket(int sock);

  Socket(const string &addr,
         high_resolution_clock::duration timeout = hours(999));

  Socket(const string &addr, uint16_t port,
         high_resolution_clock::duration timeout = hours(999));

  ~Socket();

  size_t send(const string &data, high_resolution_clock::duration timeout);

  size_t send(const string &data);

  string recv(high_resolution_clock::duration timeout);

  string recv(size_t len, high_resolution_clock::duration timeout);

  string recv(size_t len);

  string recv_until(const string &delm,
                    high_resolution_clock::duration timeout);

  Socket &setBlock();

  Socket &setNonBlock();

  bool isBlocked() const;

  operator bool();

  int getFd() const;

  bool isConnected();

  void close();

  static void close(int sock);
  bool operator==(const Socket &s) const;

  operator int() const;

private:
  Socket &setPort(uint16_t port);

  Socket &setAddr(const char *ipport);

  Socket &connect(high_resolution_clock::duration timeout);

  Socket &setBlock(bool f);

  int getSockError();

  bool isblocked_ = true;
  int sock_ = -1;
  uint16_t port_ = 0;
  string ip_;
};

Socket::Socket() {}

Socket::Socket(int sock) { sock_ = sock; }

Socket::Socket(const string &addr, high_resolution_clock::duration timeout) {
  setAddr(addr.c_str()).connect(timeout);
}

Socket::Socket(const string &addr, uint16_t port,
               high_resolution_clock::duration timeout) {
  setAddr(addr.c_str()).setPort(port).connect(timeout);
}

Socket::~Socket() { close(); }

size_t Socket::send(const string &data,
                    high_resolution_clock::duration timeout) {
  size_t sendn = 0;

  for (auto this_time = high_resolution_clock::now(), begin_time = this_time;
       sendn < data.size() && this_time - begin_time <= timeout;
       this_time = high_resolution_clock::now()) {
    auto n = ::send(sock_, data.c_str() + sendn, int(data.size() - sendn), 0);

    if (n > 0)
      sendn += n;

    else if (n == 0)
      break;

    else {
      auto e = Error::lastNetError();

      if (e == Error::wouldblock || e == Error::intr ||
          e == Error::inprogress) {
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

size_t Socket::send(const string &data) {
  size_t sendn = 0;

  while (sendn < data.size()) {
    auto n = ::send(sock_, data.c_str() + sendn, int(data.size() - sendn), 0);

    if (n > 0)
      sendn += n;

    else if (n == 0)
      break;

    else {
      auto e = Error::lastNetError();

      if (e == Error::wouldblock || e == Error::intr ||
          e == Error::inprogress) {
        continue;
      }

      break;
    }
  }
  return sendn;
}

string Socket::recv(high_resolution_clock::duration timeout) {
  string data;

  size_t buflen = 4096;

  auto buf = new_unique<char[]>(buflen);

  for (auto begin_time = high_resolution_clock::now(), this_time = begin_time;
       this_time - begin_time <= timeout;
       this_time = high_resolution_clock::now()) {
    auto n = ::recv(sock_, buf.get(), (int)buflen, 0);

    if (n > 0)
      data.append(buf.get(), n);

    else if (n == 0)
      break;

    else {
      auto e = Error::lastNetError();

      if (e == Error::wouldblock || e == Error::intr ||
          e == Error::inprogress) {
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

string Socket::recv(size_t len, high_resolution_clock::duration timeout) {
  if (len == 0)
    return "";

  size_t recvn = 0;

  auto buf = new_unique<char[]>(len);

  for (auto this_time = high_resolution_clock::now(), begin_time = this_time;
       recvn < len && this_time - begin_time <= timeout;
       this_time = high_resolution_clock::now()) {
    auto n = ::recv(sock_, buf.get() + recvn, int(len - recvn), 0);

    if (n > 0) {
      recvn += n;
    }

    else if (n == 0)
      break;

    else {
      auto e = Error::lastNetError();
      if (e == Error::wouldblock || e == Error::intr ||
          e == Error::inprogress) {
        auto sel = new_shared<Detector>();
        sel->add(this);
        sel->wait(timeout - (this_time - begin_time));
        continue;
      }
      break;
    }
  }
  return string(buf.get(), recvn);
}

string Socket::recv(size_t len) {
  if (len == 0)
    return "";

  size_t recvn = 0;

  auto buf = new_unique<char[]>(len);

  while (recvn < len) {
    auto n = ::recv(sock_, buf.get() + recvn, int(len - recvn), 0);

    if (n > 0) {
      recvn += n;
    }

    else if (n == 0)
      break;

    else {
      auto e = Error::lastNetError();
      if (e == Error::wouldblock || e == Error::intr ||
          e == Error::inprogress) {
        continue;
      }
      break;
    }
  }
  return string(buf.get(), recvn);
}

string Socket::recv_until(const string &delm,
                          high_resolution_clock::duration timeout) {
  if (delm.empty())
    return recv(timeout);

  size_t recvn = delm.size();

  string ret;

  auto buf = new_unique<char[]>(recvn);

  for (auto this_time = system_clock::now(), begin_time = this_time;
       (ret.size() < delm.size() ||
        ret.rfind(delm, ret.size() - delm.size()) == ret.npos) &&
       this_time - begin_time <= timeout;
       this_time = system_clock::now()) {
    auto n = ::recv(sock_, buf.get(), int(recvn), 0);

    if (n == 0)
      break;

    if (n < 0) {
      auto e = Error::lastNetError();

      if (e == Error::wouldblock || e == Error::intr ||
          e == Error::inprogress) {
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

Socket &Socket::setBlock() { return setBlock(true); }

Socket &Socket::setNonBlock() { return setBlock(false); }

bool Socket::isBlocked() const { return isblocked_; }

Socket::operator bool() { return isConnected(); }

int Socket::getFd() const { return sock_; }

bool Socket::isConnected() {
  auto isblocked = isblocked_;
  MYSPACE_DEFER(setBlock(isblocked));
again:
  char c;
  setNonBlock();
  auto n = ::recv(sock_, &c, 1, MSG_PEEK);
  if (n == 0)
    return false;
  if (n > 0)
    return true;
  auto e = Error::lastNetError();

  switch (e) {
  case Error::wouldblock:
  case Error::inprogress:
    return true;
  case Error::intr:
    goto again;
  default:
    return false;
  }
}

void Socket::close() {
  if (sock_ >= 0) {
    setBlock(true);
    Socket::close(sock_);
    sock_ = -1;
  }
}

void Socket::close(int sock) {
#ifdef MYSPACE_WINDOWS
  ::closesocket(sock);
#else
  ::close(sock);
#endif
}

bool Socket::operator==(const Socket &s) const { return sock_ == s.sock_; }

Socket::operator int() const { return sock_; }

Socket &Socket::setPort(uint16_t port) {
  port_ = port;
  return *this;
}

Socket &Socket::setAddr(const char *ipport) {
  auto tokens = Strings::split(ipport, ':');

  if (tokens.size() == 1) {
    ip_ = Strings::strip(tokens[0]);
  } else if (tokens.size() >= 2) {
    ip_ = Strings::strip(tokens[0]);
    port_ = StringStream(Strings::strip(tokens[1]));
  }
  return *this;
}

Socket &Socket::connect(high_resolution_clock::duration timeout) {
  if (!ip_.empty() || port_ != 0) {

    this->close();

    sock_ = (int)::socket(AF_INET, SOCK_STREAM, 0);

    setNonBlock();

    for (auto this_time = high_resolution_clock::now(), begin_time = this_time;
         !isConnected() && this_time - begin_time <= timeout;
         this_time = high_resolution_clock::now()) {

      sockaddr_in addr = {0};
      addr.sin_family = AF_INET;
      addr.sin_port = htons(port_);
      inet_pton(AF_INET, ip_.c_str(), &addr.sin_addr.s_addr);

      auto n = ::connect(sock_, (sockaddr *)&addr, sizeof(addr));

      if (n == 0)
        return *this;

      auto e = Error::lastNetError();

      switch (e) {
      case Error::isconn:
        return *this;
      case Error::already:
      case Error::inprogress:
      case Error::wouldblock:
      case Error::intr: {
        if (isConnected())
          return *this;

        auto sel = new_shared<Detector>();

        sel->add(this);

        sel->wait(timeout - (this_time - begin_time));
        break;
      }
      default: {
        this->close();
        sock_ = (int)::socket(AF_INET, SOCK_STREAM, 0);
        setNonBlock();
        this_thread::sleep_for(std::min(
            milliseconds(100),
            duration_cast<milliseconds>(timeout - (this_time - begin_time))));
        break;
      }
      }
    }
  }
  return *this;
}

Socket &Socket::setBlock(bool f) {
  SocketOpt::setBlock(sock_, f);
  isblocked_ = f;
  return *this;
}

int Socket::getSockError() {
  int err = 0;
#if defined(MYSPACE_WINDOWS)
  int len = sizeof(err);
#else
  socklen_t len = sizeof(err);
#endif
  ::getsockopt(sock_, SOL_SOCKET, SO_ERROR, (char *)&err, &len);
  return err;
}

MYSPACE_END
