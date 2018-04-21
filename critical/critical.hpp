#pragma once

#include "myspace/_/stdafx.hpp"

MYSPACE_BEGIN

template <class Hold, class Mtx = std::mutex,
          class Cond = std::condition_variable>
class Critical;

template <class Hold, class Mtx, class Cond, class X>
class _CriticalImpl : public Hold, public Mtx, public Cond {
public:
  template <class... Targs>
  _CriticalImpl(Targs &&... args) : Hold(std::forward<Targs>(args)...) {}
};

template <class Hold, class Mtx, class Cond>
class _CriticalImpl<
    Hold, Mtx, Cond,
    typename std::enable_if<std::is_fundamental<Hold>::value, Hold>::type>
    : public Mtx, public Cond {
public:
  template <class... Targs>
  _CriticalImpl(Targs &&... args) : hold_(std::forward<Targs>(args)...) {}

#if defined(MYSPACE_WINDOWS)
  template <class X>
  typename std::enable_if<!std::is_same<X, GUID>::value, bool>::type
  operator==(const Hold &other) const {
    return hold_ == other;
  }
#else
  template <class X> bool operator==(const X &other) const {
    return hold_ == other;
  }
#endif

  bool operator<(const Hold &other) const { return hold_ < other; }

  void operator--() { --hold_; }

  void operator++() { ++hold_; }

  operator const Hold &() const { return hold_; }

protected:
  Hold hold_;
};

template <class Hold, class Mtx, class Cond>
class Critical : public _CriticalImpl<Hold, Mtx, Cond, Hold> {
public:
  template <class... Targs>
  Critical(Targs &&... args)
      : _CriticalImpl<Hold, Mtx, Cond, Hold>(std::forward<Targs>(args)...) {}
};

MYSPACE_END
