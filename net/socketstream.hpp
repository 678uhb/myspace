
#pragma once

#include "myspace/_/stdafx.hpp"
#include "myspace/codec/codec.hpp"
#include "myspace/exception/exception.hpp"
#include "myspace/net/udp/socket.hpp"
#include "myspace/time/time.hpp"

MYSPACE_BEGIN

template <class SockType> class SocketStream {
public:
  MYSPACE_EXCEPTION_DEFINE(SocketStreamError, myspace::Exception)
  MYSPACE_EXCEPTION_DEFINE(SocketError, SocketStreamError)
  MYSPACE_EXCEPTION_DEFINE(TimeOut, SocketStreamError)

public:
  SocketStream(const Addr &addr);

  SocketStream(const Addr &addr,
               std::chrono::high_resolution_clock::duration timeout);

  SocketStream(std::shared_ptr<SockType> sock);

  SocketStream(std::shared_ptr<SockType> sock,
               std::chrono::high_resolution_clock::duration timeout);
  ~SocketStream();

  void setBigEndian();
  void setLittleEndian();

  /** recv **/
  // integer like
  template <class X> SocketStream &operator>>(X &x) noexcept(false);
  template <class X> X recv() noexcept(false);
  template <class X> X peek() noexcept(false);
  template <class X> void recv(X &x) noexcept(false);
  template <class X> void peek(X &x) noexcept(false);
  std::string recvUntil(const std::string &token) noexcept(false);
  // std::string like
  template <class X> X recv(size_t len) noexcept(false);
  template <class X> X peek(size_t len) noexcept(false);
  template <class X> void recv(X &x, size_t len) noexcept(false);
  template <class X> void peek(X &x, size_t len) noexcept(false);
  size_t recvSize() const;

  /** send **/
  // sizeable
  // hold : append to send buffer , but no write, until next call send, or
  //        flush, or destructor
  // send : append to send buffer, and send the buffer
  template <class X> SocketStream &operator<<(const X &x) noexcept(false);
  template <class X> void send(const X &x) noexcept(false);
  template <class X> void hold(const X &x) noexcept(false);
  template <class X, size_t N> void send(const X (&x)[N]) noexcept(false);
  template <class X, size_t N> void hold(const X (&x)[N]) noexcept(false);
  // unsizeable
  template <class X> void send(const X &x, size_t len) noexcept(false);
  template <class X> void hold(const X &x, size_t len) noexcept(false);
  void flush();
  size_t holdSize() const;

private:
  template <class X> void fetch(X &x, bool peek, size_t len) noexcept(false);
  template <class X> X &toX(X &x, size_t len);
  template <class X> size_t getLen(size_t len);
  void purchase(size_t len);

private:
  bool big_endian_ = true;
  bool use_timeout_ = false;
  std::chrono::high_resolution_clock::duration timeout_;
  std::shared_ptr<SockType> sock_;
  std::string recvstr_;
  std::string sendstr_;
};

namespace socketstreamimpl {

template <class X> inline size_t getLen(size_t len) { return sizeof(X); }

template <> inline size_t getLen<std::string>(size_t len) { return len; }

template <class X>
inline X &toX(X &x, size_t, bool t_big_endian, const std::string &buffer) {
  if (t_big_endian) {
    x = Codec::ntoh(*(X *)buffer.c_str());
  } else {
    x = *(X *)buffer.c_str();
  }
  return x;
}

template <>
inline std::string &toX<std::string>(std::string &x, size_t len, bool,
                                     const std::string &buffer) {
  x = buffer.substr(0, len);
  return x;
}

template <>
inline uint8_t &toX<uint8_t>(uint8_t &x, size_t len, bool,
                             const std::string &buffer) {
  x = buffer[0];
  return x;
}

template <>
inline int8_t &toX<int8_t>(int8_t &x, size_t len, bool,
                           const std::string &buffer) {
  x = buffer[0];
  return x;
}

template <class X> inline std::string toString(const X &x, bool t_big_endian) {
  std::string result;
  if (t_big_endian) {
    auto y = Codec::hton(x);
    result.append((const char *)&y, sizeof(y));
  } else {
    result.append((const char *)&x, sizeof(x));
  }
  return result;
}

inline const std::string &toString(const std::string &x, bool) { return x; }
} // namespace socketstreamimpl

template <class SockType>
inline void SocketStream<SockType>::setLittleEndian() {
  big_endian_ = false;
}

template <class SockType> inline void SocketStream<SockType>::setBigEndian() {
  big_endian_ = true;
}

template <class SockType>
inline SocketStream<SockType>::SocketStream(std::shared_ptr<SockType> sock)
    : sock_(sock) {}

template <class SockType>
inline SocketStream<SockType>::SocketStream(
    std::shared_ptr<SockType> sock,
    std::chrono::high_resolution_clock::duration timeout)
    : use_timeout_(true), timeout_(timeout), sock_(sock) {}

template <class SockType>
inline SocketStream<SockType>::SocketStream(const Addr &addr)
    : use_timeout_(false), sock_(new_shared<SockType>(addr)) {}

template <class SockType>
inline SocketStream<SockType>::SocketStream(
    const Addr &addr, std::chrono::high_resolution_clock::duration timeout)
    : use_timeout_(true), timeout_(timeout),
      sock_(new_shared<SockType>(addr, timeout)) {}

template <class SockType>
template <class X>
SocketStream<SockType> &SocketStream<SockType>::
operator>>(X &x) noexcept(false) {
  fetch<X>(x, false, sizeof(X));
  return *this;
}

template <class SockType>
template <class X>
inline X SocketStream<SockType>::recv() noexcept(false) {
  X x;
  fetch<X>(x, false, 0);
  return std::move(x);
}

template <class SockType>
template <class X>
inline X SocketStream<SockType>::peek() noexcept(false) {
  X x;
  fetch<X>(x, true, 0);
  return std::move(x);
}

template <class SockType>
template <class X>
inline X SocketStream<SockType>::recv(size_t len) noexcept(false) {
  X x;
  fetch<X>(x, false, len);
  return std::move(x);
}

template <class SockType>
template <class X>
inline X SocketStream<SockType>::peek(size_t len) noexcept(false) {
  std::string x;
  fetch(x, true, len);
  return std::move(x);
}

template <class SockType>
template <class X>
inline void SocketStream<SockType>::recv(X &x, size_t len) noexcept(false) {
  fetch(x, false, len);
}

template <class SockType>
template <class X>
inline void SocketStream<SockType>::peek(X &x, size_t len) noexcept(false) {
  fetch(x, true, len);
}

template <class SockType>
template <class X>
inline void SocketStream<SockType>::fetch(X &x, bool peek,
                                          size_t len) noexcept(false) {
  size_t x_len = getLen<X>(len);
  if (recvstr_.size() < x_len) {
    purchase(x_len - recvstr_.size());
  }
  MYSPACE_THROW_IF_EX(SocketStream::SocketError, recvstr_.size() < x_len);
  MYSPACE_DEFER(if (!peek) recvstr_.erase(0, x_len););
  toX<X>(x, x_len);
}

template <class SockType>
inline void SocketStream<SockType>::purchase(size_t want) {
  if (use_timeout_) {
    timeout_ -=
        Time::costs([&]() { recvstr_.append(sock_->recv(want, timeout_)); });
    MYSPACE_THROW_IF_EX(SocketStream::TimeOut, recvstr_.size() < want);
  } else {
    recvstr_.append(sock_->recv(want));
  }
}

template <> inline void SocketStream<udp::Socket>::purchase(size_t want) {
  if (use_timeout_) {
    timeout_ -= Time::costs([&]() { recvstr_.append(sock_->recv(timeout_)); });
    // MYSPACE_THROW_IF_EX(SocketStream::TimeOut, recvstr_.size() < want);
  } else {
    recvstr_.append(sock_->recv());
  }
}

template <class SockType>
template <class X>
inline size_t SocketStream<SockType>::getLen(size_t len) {
  return socketstreamimpl::getLen<X>(len);
}

template <class SockType>
template <class X>
inline X &SocketStream<SockType>::toX(X &x, size_t len) {
  return socketstreamimpl::toX(x, len, big_endian_, recvstr_);
}

template <class SockType>
template <class X>
inline void SocketStream<SockType>::send(const X &x) noexcept(false) {
  if (!use_timeout_)
    sock_->send(socketstreamimpl::toString(x, big_endian_));
  else {
    timeout_ -= Time::costs([&]() {
      sock_->send(socketstreamimpl::toString(x, big_endian_), timeout_);
    });
  }
}

template <class SockType> inline void SocketStream<SockType>::flush() {
  size_t sendn = 0;
  if (use_timeout_) {
    timeout_ -= Time::costs([&]() { sendn = sock_->send(sendstr_, timeout_); });
    MYSPACE_THROW_IF_EX(TimeOut, sendn != sendstr_.size());
  } else {
    sendn = sock_->send(sendstr_);
  }
  sendstr_.erase(0, sendn);
}

template <class SockType>
template <class X>
inline void SocketStream<SockType>::hold(const X &x) noexcept(false) {
  sendstr_.append(socketstreamimpl::toString(x, big_endian_));
}

template <class SockType>
inline size_t SocketStream<SockType>::recvSize() const {
  return recvstr_.size();
}

template <class SockType>
inline size_t SocketStream<SockType>::holdSize() const {
  return sendstr_.size();
}
template <class SockType> inline SocketStream<SockType>::~SocketStream() {
  try {
    flush();
  } catch (...) {
    MYSPACE_DEV_EXCEPTION();
  }
}

template <class SockType>
inline std::string
SocketStream<SockType>::recvUntil(const std::string &token) noexcept(false) {
  std::string tmp;
  auto lesscount = Strings::endWithLess(recvstr_, token);
  if (lesscount > 0) {
    if (use_timeout_) {
      timeout_ -= Time::costs([&]() {
        recvstr_.append(sock_->recvUntil(
            token.substr(token.size() - lesscount, lesscount), timeout_));
      });
    } else {
      recvstr_.append(
          sock_->recvUntil(token.substr(token.size() - lesscount, lesscount)));
    }
  }
  MYSPACE_THROW_IF_EX(TimeOut, !Strings::endWith(recvstr_, token));
  tmp.swap(recvstr_);
  return tmp;
}

MYSPACE_END
