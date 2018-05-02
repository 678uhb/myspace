
#pragma once

#include "myspace/_/stdafx.hpp"

#if defined(MYSPACE_LINUX)

#include "myspace/any/any.hpp"
#include "myspace/detector/_/include.hpp"
#include "myspace/memory/memory.hpp"
#include "myspace/net/socketopt.hpp"

MYSPACE_BEGIN

class Epoll {
public:
  Epoll();

  ~Epoll();

  template <class X> bool add(X &&x, DetectType dt = READ);

  template <class X> bool mod(X &&x, DetectType dt);

  template <class X> bool aod(X &&x, DetectType dt = READ);

  template <class X> void del(X &&x);

  std::map<uint32_t, std::deque<Any> > wait();

  std::map<uint32_t, std::deque<Any> >
  wait(std::chrono::high_resolution_clock::duration duration);

private:
  std::map<uint32_t, std::deque<Any> > wait_ms(int ms);

  int epoll_ = -1;

  std::unordered_map<int, DetectorImpl::Candidate> candidates_;
};
inline Epoll::Epoll() : epoll_(epoll_create1(0)) {}
inline Epoll::~Epoll() {
  ::close(epoll_);
  epoll_ = -1;
}

template <class X> inline bool Epoll::add(X &&x, DetectType dt) {
  if (!x)
    return false;

  int fd = x->operator int();

  if (candidates_.find(fd) != candidates_.end()) {
    return false;
  }

  epoll_event ev;
  ev.events = dt;
  ev.data.fd = fd;

  SocketOpt::setBlock(fd, false);

  if (0 == ::epoll_ctl(epoll_, EPOLL_CTL_ADD, fd, &ev)) {
    candidates_[fd] = { Any(x), dt };

    return true;
  }
  return false;
}

template <class X> inline bool Epoll::mod(X &&x, DetectType dt) {
  if (!x)
    return false;

  int fd = x->operator int();

  auto itr = candidates_.find(fd);

  if (itr == candidates_.end())
    return false;

  if (itr->second._ev == dt)
    return true;

  epoll_event ev;
  ev.events = dt;
  ev.data.fd = fd;

  SocketOpt::setBlock(fd, false);

  if (0 == ::epoll_ctl(epoll_, EPOLL_CTL_MOD, fd, &ev)) {
    itr->second = { Any(x), dt };

    return true;
  }
  return false;
}

template <class X> inline bool Epoll::aod(X &&x, DetectType dt) {
  return add(x, dt) || mod(x, dt);
}

template <class X> inline void Epoll::del(X &&x) {
  if (!x)
    return;

  int fd = x->operator int();

  epoll_event ev;
  ::epoll_ctl(epoll_, EPOLL_CTL_DEL, fd, &ev);

  candidates_.erase(fd);
}
inline std::map<uint32_t, std::deque<Any> > Epoll::wait() {
  return std::move(wait_ms(-1));
}
inline std::map<uint32_t, std::deque<Any> >
Epoll::wait(std::chrono::high_resolution_clock::duration duration) {
  auto ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

  if (ms < 0) {
    ms = 0;
  }

  return std::move(wait_ms(ms));
}
inline std::map<uint32_t, std::deque<Any> > Epoll::wait_ms(int ms) {
  std::map<uint32_t, std::deque<Any> > result;

  auto events = newUnique<epoll_event[]>(candidates_.size());

  auto n = ::epoll_wait(epoll_, events.get(), candidates_.size(), ms);

  for (auto i = 0; i < n; ++i) {
    auto ev = events[i].events;

    auto fd = events[i].data.fd;

    result[ev].push_back(candidates_[fd]._x);
  }

  return std::move(result);
}

MYSPACE_END

#endif
