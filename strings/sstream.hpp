
#pragma once

#include "myspace/_/stdafx.hpp"

MYSPACE_BEGIN

class StringStream : public std::stringstream {
public:
  template <class... Targs> StringStream(Targs &&... args);

  template <class Type> StringStream &operator<<(const Type &x);

  template <class Type> StringStream &operator>>(Type &x);

  template <class Type> operator Type();

  std::string str();

  template <class... Targs> StringStream &put(Targs &&... args);

private:
  template <class T, class... Targs>
  StringStream &_put(T &&x, Targs &&... args);

  template <class T> StringStream &_put(T &&x);

  StringStream &_put();

private:
  std::stringstream ss_;
};

template <class... Targs> inline StringStream::StringStream(Targs &&... args) {
  put(std::forward<Targs>(args)...);
}

template <class Type>
inline StringStream &StringStream::operator<<(const Type &x) {
  ss_ << x;
  return *this;
}

template <class Type> inline StringStream &StringStream::operator>>(Type &x) {
  ss_ >> x;
  return *this;
}

template <class Type> inline StringStream::operator Type() {
  Type x;
  ss_ >> x;
  return x;
}

inline std::string StringStream::str() { return ss_.str(); }

template <class... Targs>
inline StringStream &StringStream::put(Targs &&... args) {
  _put(std::forward<Targs>(args)...);
  return *this;
}

template <class T, class... Targs>
inline StringStream &StringStream::_put(T &&x, Targs &&... args) {
  ss_ << x;
  return _put(std::forward<Targs>(args)...);
}

template <class T> inline StringStream &StringStream::_put(T &&x) {
  ss_ << x;
  return *this;
}

inline StringStream &StringStream::_put() { return *this; }

MYSPACE_END
