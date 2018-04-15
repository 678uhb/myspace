
#pragma once

#include "myspace/_/stdafx.hpp"
#include "myspace/detector/_/include.hpp"

MYSPACE_BEGIN

class Select {
public:
  template <class X> bool add(X &&x, DetectType dt = READ);

  template <class X> bool mod(X &&x, DetectType ev);

  template <class X> bool aod(X &&x, DetectType ev = READ);

  template <class X> void del(X &&x);

  std::map<uint32_t, std::deque<Any>> wait();

  std::map<uint32_t, std::deque<Any>>
  wait(std::chrono::high_resolution_clock::duration duration);

private:
  std::map<uint32_t, std::deque<Any>> wait_tv(timeval *ptv);

  std::unordered_map<int, DetectorImpl::Candidate> candidates_;

  // fd_set could be very large, according to FD_SETSIZE
  std::unique_ptr<fd_set> r_ = new_unique<fd_set>();
  std::unique_ptr<fd_set> w_ = new_unique<fd_set>();
  std::unique_ptr<fd_set> e_ = new_unique<fd_set>();
};

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
inline std::map<uint32_t, std::deque<Any>> Select::wait() {
  return wait_tv(nullptr);
}
inline std::map<uint32_t, std::deque<Any>>
Select::wait(std::chrono::high_resolution_clock::duration duration) {
  if (std::chrono::duration_cast<std::chrono::microseconds>(duration).count() <
      0) {
    duration = std::chrono::microseconds(0);
  }

  timeval tv{
      (long)std::chrono::duration_cast<std::chrono::seconds>(duration).count(),
      (long)std::chrono::duration_cast<std::chrono::microseconds>(duration)
              .count() %
          1000000};

  return wait_tv(&tv);
}
inline std::map<uint32_t, std::deque<Any>> Select::wait_tv(timeval *ptv) {
  FD_ZERO(r_.get());
  FD_ZERO(w_.get());
  FD_ZERO(e_.get());

  int maxfd = 0;

  std::map<uint32_t, std::deque<Any>> result;

  for (auto &p : candidates_) {
    auto &fd = p.first;

    auto &candidate = p.second;

    maxfd = std::max(maxfd, fd);

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

MYSPACE_END
