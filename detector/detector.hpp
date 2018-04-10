
#pragma once

#include "myspace/_/include.hpp"
#include "myspace/any/any.hpp"
#include "myspace/memory/memory.hpp"
#include "myspace/socket/socketopt.hpp"

MYSPACE_BEGIN

enum DetectType {
#ifdef MYSPACE_LINUX
  READ = EPOLLIN,
  WRITE = EPOLLOUT,
  EXCEP = EPOLLERR,
  READ_WRITE = READ | WRITE,
#else
  READ = 1,
  WRITE = (1 << 1),
  EXCEP = (1 << 2),
  READ_WRITE = READ | WRITE,
#endif
};

namespace DetectorImpl {
struct Candidate {
  Any _x;

  DetectType _ev;
};
} // namespace DetectorImpl

class Select {
public:
  template <class X> bool add(X &&x, DetectType dt = READ);

  template <class X> bool mod(X &&x, DetectType ev);

  template <class X> bool aod(X &&x, DetectType ev = READ);

  template <class X> void del(X &&x);

  map<uint32_t, deque<Any>> wait();

  map<uint32_t, deque<Any>> wait(high_resolution_clock::duration duration);

private:
  map<uint32_t, deque<Any>> wait_tv(timeval *ptv);

  unordered_map<int, DetectorImpl::Candidate> candidates_;

  // fd_set could be very large, according to FD_SETSIZE
  unique_ptr<fd_set> r_ = new_unique<fd_set>();
  unique_ptr<fd_set> w_ = new_unique<fd_set>();
  unique_ptr<fd_set> e_ = new_unique<fd_set>();
};

#ifdef MYSPACE_LINUX
class Epoll {
public:
  Epoll();

  ~Epoll();

  template <class X> bool add(X &&x, DetectType dt = READ);

  template <class X> bool mod(X &&x, DetectType dt);

  template <class X> bool aod(X &&x, DetectType dt = READ);

  template <class X> void del(X &&x);

  map<uint32_t, deque<Any>> wait();

  map<uint32_t, deque<Any>> wait(high_resolution_clock::duration duration);

private:
  map<uint32_t, deque<Any>> wait_ms(int ms);

  int epoll_ = -1;

  unordered_map<int, DetectorImpl::Candidate> candidates_;
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
    candidates_[fd] = {Any(x), dt};

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
    itr->second = {Any(x), dt};

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
inline map<uint32_t, deque<Any>> Epoll::wait() { return move(wait_ms(-1)); }
inline map<uint32_t, deque<Any>>
Epoll::wait(high_resolution_clock::duration duration) {
  auto ms = duration_cast<milliseconds>(duration).count();

  if (ms < 0) {
    ms = 0;
  }

  return move(wait_ms(ms));
}
inline map<uint32_t, deque<Any>> Epoll::wait_ms(int ms) {
  map<uint32_t, deque<Any>> result;

  auto events = new_unique<epoll_event[]>(candidates_.size());

  auto n = ::epoll_wait(epoll_, events.get(), candidates_.size(), ms);

  for (auto i = 0; i < n; ++i) {
    auto ev = events[i].events;

    auto fd = events[i].data.fd;

    result[ev].push_back(candidates_[fd]._x);
  }

  return move(result);
}

#endif

template <class X> inline bool Select::add(X &&x, DetectType dt) {
  if (candidates_.size() >= FD_SETSIZE) {
    return false;
  }

  int fd = x->operator int();

  if (candidates_.find(fd) != candidates_.end()) {
    return false;
  }

  SocketOpt::setBlock(fd, false);

  candidates_[fd] = {Any(x), dt};

  return true;
}

template <class X> inline bool Select::mod(X &&x, DetectType ev) {
  int fd = x->operator int();

  auto itr = candidates_.find(fd);

  if (itr == candidates_.end()) {
    return false;
  }

  SocketOpt::setBlock(fd, false);

  itr->second = DetectorImpl::Candidate{Any(x), ev};

  return true;
}

template <class X> inline bool Select::aod(X &&x, DetectType ev) {
  return add(x, ev) || mod(x, ev);
}

template <class X> inline void Select::del(X &&x) {
  int fd = x->operator int();

  candidates_.erase(fd);
}
inline map<uint32_t, deque<Any>> Select::wait() {
  return wait_tv(nullptr);
}
inline map<uint32_t, deque<Any>>
Select::wait(high_resolution_clock::duration duration) {
  if (duration_cast<microseconds>(duration).count() < 0) {
    duration = microseconds(0);
  }

  timeval tv{(long)duration_cast<seconds>(duration).count(),
             (long)duration_cast<microseconds>(duration).count() % 1000000};

  return wait_tv(&tv);
}
inline map<uint32_t, deque<Any>> Select::wait_tv(timeval *ptv) {
  FD_ZERO(r_.get());
  FD_ZERO(w_.get());
  FD_ZERO(e_.get());

  int maxfd = 0;

  map<uint32_t, deque<Any>> result;

  for (auto &p : candidates_) {
    auto &fd = p.first;

    auto &candidate = p.second;

    maxfd = max(maxfd, fd);

    if (candidate._ev & READ)
      FD_SET(fd, r_.get());

    if (candidate._ev & WRITE)
      FD_SET(fd, w_.get());

    if (candidate._ev & EXCEP)
      FD_SET(fd, e_.get());
  }

  auto n = ::select(maxfd + 1, r_.get(), w_.get(), e_.get(), ptv);

  if (n <= 0)
    return result;

  for (auto &p : candidates_) {
    if (n <= 0)
      break;

    auto &fd = p.first;

    auto &candidate = p.second;

    if (FD_ISSET(fd, r_.get()))
      result[READ].push_back(candidate._x), n--;

    if (FD_ISSET(fd, w_.get()))
      result[WRITE].push_back(candidate._x), n--;

    if (FD_ISSET(fd, e_.get()))
      result[EXCEP].push_back(candidate._x), n--;
  }

  return result;
}

#ifdef MYSPACE_LINUX
typedef Epoll Detector;
#else
typedef Select Detector;
#endif

MYSPACE_END
