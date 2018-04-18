
#pragma once

#include "myspace/_/stdafx.hpp"
#include "myspace/codec/codec.hpp"
#include "myspace/defer/defer.hpp"
#include "myspace/exception/exception.hpp"
MYSPACE_BEGIN

class NetStream {
public:
  MYSPACE_EXCEPTION_DEFINE(NetStreamError, myspace::Exception)
  MYSPACE_EXCEPTION_DEFINE(Insufficient, NetStreamError)

public:
  // constructor
  NetStream();
  NetStream(const std::string &);

  // get whole buffer
  std::string get();
  const std::string &peek() const;

  // access
  size_t size() const;
  bool empty() const;

  // pushBack
  NetStream &operator<<(uint8_t);
  NetStream &operator<<(uint16_t);
  NetStream &operator<<(uint32_t);
  NetStream &pushBack(uint16_t, bool reverse = true);
  template <class X> NetStream &operator<<(const X &x);

  // popFront
  NetStream &operator>>(uint8_t &) noexcept(false);
  NetStream &operator>>(uint16_t &) noexcept(false);
  NetStream &operator>>(uint32_t &) noexcept(false);
  NetStream &popFront(std::string &, size_t) noexcept(false);
  NetStream &popFront(uint16_t &, bool reverse = true) noexcept(false);

  template <class X> X popFront() noexcept(false);
  template <class X> X popFront(size_t) noexcept(false);
  template <class X> X front() const noexcept(false);
  void front(uint8_t &) const noexcept(false);
  void front(uint16_t &) const noexcept(false);

private:
  std::string packed_;
};

inline NetStream::NetStream() {}
inline NetStream::NetStream(const std::string &x) : packed_(x) {}

inline NetStream &NetStream::operator<<(uint8_t x) {
  packed_.append(1, (char)x);
  return *this;
}

inline NetStream &NetStream::operator<<(uint16_t x) {
  x = Codec::hton(x);
  packed_.append((char *)&x, sizeof(x));
  return *this;
}
inline NetStream &NetStream::pushBack(uint16_t x, bool reverse) {
  if (reverse) {
    return operator<<(x);
  }
  packed_.append((char *)&x, sizeof(x));
  return *this;
}

inline NetStream &NetStream::operator<<(uint32_t x) {
  x = Codec::hton(x);
  packed_.append((char *)&x, sizeof(x));
  return *this;
}
template <class X> inline NetStream &NetStream::operator<<(const X &x) {
  packed_.append(x);
  return *this;
}
inline size_t NetStream::size() const { return packed_.size(); }
inline bool NetStream::empty() const { return packed_.empty(); }

inline std::string NetStream::get() {
  std::string tmp;
  tmp.swap(packed_);
  return tmp;
}
inline const std::string &NetStream::peek() const { return packed_; }

inline NetStream &NetStream::
operator>>(uint8_t &x) noexcept(false) {
  MYSPACE_THROW_IF_EX(NetStream::Insufficient, packed_.empty());
  x = packed_.front();
  packed_.erase(0, 1);
  return *this;
}
inline NetStream &NetStream::
operator>>(uint16_t &x) noexcept(false) {
  MYSPACE_THROW_IF_EX(NetStream::Insufficient, packed_.size() < 2);
  x = Codec::ntoh(*(uint16_t *)(packed_.c_str()));
  packed_.erase(0, 2);
  return *this;
}
inline NetStream &NetStream::
operator>>(uint32_t &x) noexcept(false) {
  MYSPACE_THROW_IF_EX(NetStream::Insufficient, packed_.size() < sizeof(x));
  x = Codec::ntoh(*(uint32_t *)(packed_.c_str()));
  packed_.erase(0, 4);
  return *this;
}
inline NetStream &
NetStream::popFront(std::string &x, size_t len) noexcept(false) {
  MYSPACE_THROW_IF_EX(NetStream::Insufficient, packed_.size() < len);
  x = packed_.substr(0, len);
  packed_.erase(0, len);
  return *this;
}
inline NetStream &
NetStream::popFront(uint16_t &x, bool reverse) noexcept(false) {
  if (reverse) {
    return operator>>(x);
  }
  MYSPACE_THROW_IF_EX(NetStream::Insufficient, packed_.size() < sizeof(x));
  x = *(uint16_t *)packed_.c_str();
  packed_.erase(0, 2);
  return *this;
}

inline void NetStream::front(uint8_t &x) const noexcept(false) {
  MYSPACE_THROW_IF_EX(NetStream::Insufficient, packed_.empty());
  x = (uint8_t)packed_[0];
}

inline void NetStream::front(uint16_t &x) const noexcept(false) {
  MYSPACE_THROW_IF_EX(NetStream::Insufficient, packed_.size() < sizeof(x));
  x = Codec::ntoh(*(uint16_t *)packed_.c_str());
}

template <>
inline uint8_t NetStream::front<uint8_t>() const
    noexcept(false) {
  uint8_t x;
  this->front(x);
  return x;
}

template <>
inline uint16_t NetStream::front<uint16_t>() const
    noexcept(false) {
  uint16_t x;
  this->front(x);
  return x;
}

template <>
inline uint8_t NetStream::popFront<uint8_t>() noexcept(false) {
  MYSPACE_THROW_IF_EX(NetStream::Insufficient, packed_.empty());
  MYSPACE_DEFER(packed_.erase(0, 1));
  return (uint8_t)packed_[0];
}

template <>
inline uint16_t NetStream::popFront<uint16_t>() noexcept(false) {
  uint16_t x;
  popFront(x, true);
  return x;
}

template <>
inline std::string
NetStream::popFront<std::string>(size_t len) noexcept(false) {
  MYSPACE_THROW_IF_EX(NetStream::Insufficient, packed_.size() < len);
  MYSPACE_DEFER(packed_.erase(0, len));
  return packed_.substr(0, len);
}
MYSPACE_END
