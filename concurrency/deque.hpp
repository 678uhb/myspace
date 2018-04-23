
#pragma once
#include "myspace/_/stdafx.hpp"
#include "myspace/concurrency/exception.hpp"
MYSPACE_BEGIN

namespace concurrency {

template <class X> class Deque {
public:
  void pushFront(const X &x);

  template <class Predicate> void pushFront(const X &x, Predicate pred);

  template <class Rep, class Period>
  void pushFrontFor(
      const X &x,
      const std::chrono::duration<Rep, Period> &timeout) noexcept(false);

  template <class Rep, class Period, class Predicate>
  void pushFrontFor(const X &x,
                    const std::chrono::duration<Rep, Period> &timeout,
                    Predicate pred) noexcept(false);

  void pushBack(const X &x);

  template <class Predicate> void pushBack(const X &x, Predicate pred);

  template <class Rep, class Period>
  void pushBackFor(
      const X &x,
      const std::chrono::duration<Rep, Period> &timeout) noexcept(false);

  template <class Rep, class Period, class Predicate>
  void pushBackFor(const X &x,
                   const std::chrono::duration<Rep, Period> &timeout,
                   Predicate pred) noexcept(false);

  X popFront();

  template <class Predicate> X popFront(Predicate pred);

  template <class Rep, class Period>
  X popFrontFor(const std::chrono::duration<Rep, Period> &timeout) noexcept(
      false);

  template <class Rep, class Period, class Predicate>
  X popFrontFor(const std::chrono::duration<Rep, Period> &timeout,
                Predicate pred) noexcept(false);

  X popBack();

  template <class Predicate> X popBack(Predicate pred);

  template <class Rep, class Period>
  X popBackFor(const std::chrono::duration<Rep, Period> &timeout) noexcept(
      false);

  template <class Rep, class Period, class Predicate>
  X popBackFor(const std::chrono::duration<Rep, Period> &timeout,
               Predicate pred) noexcept(false);

  bool empty();
  size_t size();
  size_t maxSize() const;

private:
  void push(bool front, const X &);

  template <class Predicate> void push(bool front, const X &x, Predicate pred);

  template <class Rep, class Period>
  void pushFor(bool front, const X &x,
               const std::chrono::duration<Rep, Period> &timeout);

  template <class Rep, class Period, class Predicate>
  void pushFor(bool front, const X &x,
               const std::chrono::duration<Rep, Period> &timeout,
               Predicate pred);

  X pop(bool front);

  template <class Predicate> X pop(bool front, Predicate pred);

  template <class Rep, class Period>
  X popFor(bool front, const std::chrono::duration<Rep, Period> &timeout);

  template <class Rep, class Period, class Predicate>
  X popFor(bool front, const std::chrono::duration<Rep, Period> &timeout,
           Predicate pred);

private:
  Deque(size_t max_size);
  size_t max_size_ = 0;
  std::recursive_mutex mtx_;
  std::condition_variable_any cond_;
  std::deque<X> deque_;

  friend class concurrency::Factory;
};

template <class X>
template <class Predicate>
inline void Deque<X>::pushFront(const X &x, Predicate pred) {
  push(true, x, pred);
}

template <class X>
template <class Predicate>
inline void Deque<X>::pushBack(const X &x, Predicate pred) {
  pop(false, x, pred);
}

template <class X>
template <class Predicate>
inline X Deque<X>::popFront(Predicate pred) {
  return pop(true, pred);
}

template <class X>
template <class Predicate>
inline X Deque<X>::popBack(Predicate pred) {
  return pop(false, pred);
}

template <class X> inline X Deque<X>::popFront() { return pop(true); }

template <class X>
template <class Rep, class Period>
inline X Deque<X>::popFrontFor(
    const std::chrono::duration<Rep, Period> &timeout) noexcept(false) {
  return popFor(true, timeout);
}

template <class X>
template <class Rep, class Period, class Predicate>
inline X
Deque<X>::popFrontFor(const std::chrono::duration<Rep, Period> &timeout,
                      Predicate pred) noexcept(false) {
  return popFor(true, timeout, pred);
}

template <class X> inline X Deque<X>::popBack() { return pop(false); }

template <class X>
template <class Rep, class Period>
inline X Deque<X>::popBackFor(
    const std::chrono::duration<Rep, Period> &timeout) noexcept(false) {
  return popFor(false, timeout);
}

template <class X>
template <class Rep, class Period, class Predicate>
inline X Deque<X>::popBackFor(const std::chrono::duration<Rep, Period> &timeout,
                              Predicate pred) noexcept(false) {
  return popFor(false, timeout, pred);
}

template <class X> inline void Deque<X>::pushFront(const X &x) {
  push(true, x);
}

template <class X>
template <class Rep, class Period>
inline void Deque<X>::pushFrontFor(
    const X &x,
    const std::chrono::duration<Rep, Period> &timeout) noexcept(false) {
  pushFor(true, x, timeout);
}

template <class X>
template <class Rep, class Period, class Predicate>
inline void
Deque<X>::pushFrontFor(const X &x,
                       const std::chrono::duration<Rep, Period> &timeout,
                       Predicate pred) noexcept(false) {
  pushFor(true, x, timeout, pred);
}

template <class X> inline void Deque<X>::pushBack(const X &x) {
  push(false, x);
}

template <class X>
template <class Rep, class Period>
inline void Deque<X>::pushBackFor(
    const X &x,
    const std::chrono::duration<Rep, Period> &timeout) noexcept(false) {
  pushFor(false, x, timeout);
}

template <class X>
template <class Rep, class Period, class Predicate>
inline void
Deque<X>::pushBackFor(const X &x,
                      const std::chrono::duration<Rep, Period> &timeout,
                      Predicate pred) noexcept(false) {
  push(false, x, timeout, pred);
}

template <class X>
inline Deque<X>::Deque(size_t max_size) : max_size_(max_size) {}

template <class X> inline void Deque<X>::push(bool front, const X &x) {
  for (auto ul = std::unique_lock<std::recursive_mutex>(mtx_);;) {
    if (deque_.size() < maxSize()) {
      if (front)
        deque_.push_front(x);
      else
        deque_.push_back(x);
      break;
    } else {
      cond_.wait(ul, [&]() { return deque_.size() < maxSize(); });
    }
  }
  cond_.notify_one();
}

template <class X>
template <class Predicate>
inline void Deque<X>::push(bool front, const X &x, Predicate pred) {
  for (auto ul = std::unique_lock<std::recursive_mutex>(mtx_); !pred();) {
    if (deque_.size() < maxSize()) {
      MYSPACE_DEFER(cond_.notify_one());
      if (front)
        deque_.push_front(x);
      else
        deque_.push_back(x);
      return;
    } else {
      cond_.wait(ul, [&]() { return deque_.size() < maxSize() || pred(); });
    }
  }
  MYSPACE_THROW_EX(PredicateMeet);
}

template <class X>
template <class Rep, class Period>
void Deque<X>::pushFor(bool front, const X &x,
                       const std::chrono::duration<Rep, Period> &timeout) {
  auto begin_time = high_resolution_clock::now();
  for (auto ul = std::unique_lock<std::recursive_mutex>(mtx_);;) {
    if (deque_.size() < maxSize()) {
      if (front)
        deque_.push_front(x);
      else
        deque_.push_back(x);
      ul.unlock();
      break;
    } else {
      auto now = high_resolution_clock::now();
      MYSPACE_THROW_IF_EX(concurrency::TimeOut, now - begin_time > timeout);
      cond_.wait_for(ul, now - begin_time,
                     [&]() { return deque_.size() < maxSize(); });
    }
  }
  cond_.notify_one();
}
template <class X>
template <class Rep, class Period, class Predicate>
void Deque<X>::pushFor(bool front, const X &x,
                       const std::chrono::duration<Rep, Period> &timeout,
                       Predicate pred) {
  auto begin_time = high_resolution_clock::now();
  for (auto ul = std::unique_lock<std::recursive_mutex>(mtx_); !pred();) {
    if (deque_.size() < maxSize()) {
      if (front)
        deque_.push_front(x);
      else
        deque_.push_back(x);
      ul.unlock();
      break;
    } else {
      auto now = high_resolution_clock::now();
      MYSPACE_THROW_IF_EX(concurrency::TimeOut, now - begin_time > timeout);
      cond_.wait_for(ul, now - begin_time,
                     [&]() { return deque_.size() < maxSize() || pred(); });
    }
  }
  cond_.notify_one();
}

template <class X> inline X Deque<X>::pop(bool front) {
  MYSPACE_DEFER(cond_.notify_one());
  for (auto ul = std::unique_lock<std::recursive_mutex>(mtx_);;) {
    if (!deque_.empty()) {
      MYSPACE_DEFER(if (front) deque_.pop_front(); else deque_.pop_back(););
      return (front ? deque_.front() : deque_.back());
    } else {
      cond_.wait_for(ul, [&]() { return !deque_.empty(); });
    }
  }
  throw;
}

template <class X>
template <class Predicate>
inline X Deque<X>::pop(bool front, Predicate pred) {
  MYSPACE_DEFER(cond_.notify_one());
  for (auto ul = std::unique_lock<std::recursive_mutex>(mtx_); !pred();) {
    if (!deque_.empty()) {
      MYSPACE_DEFER(if (front) deque_.pop_front(); else deque_.pop_back(););
      return (front ? deque_.front() : deque_.back());
    } else {
      cond_.wait(ul, [&]() { return !deque_.empty() || pred(); });
    }
  }
  MYSPACE_THROW_EX(PredicateMeet);
  throw;
}

template <class X>
template <class Rep, class Period>
inline X Deque<X>::popFor(bool front,
                          const std::chrono::duration<Rep, Period> &timeout) {
  auto begin_time = high_resolution_clock::now();
  for (auto ul = std::unique_lock<std::recursive_mutex>(mtx_);;) {
    if (!deque_.empty()) {
      MYSPACE_DEFER(cond_.notify_one());
      MYSPACE_DEFER(if (front) deque_.pop_front(); else deque_.pop_back(););
      return (front ? deque_.front() : deque_.back());
    } else {
      auto now = high_resolution_clock::now();
      MYSPACE_THROW_IF_EX(concurrency::TimeOut, now - begin_time > timeout);
      cond_.wait_for(ul, now - begin_time, [&]() { return !deque_.empty(); });
    }
  }
  MYSPACE_THROW_EX(concurrency::TimeOut);
  throw;
}
template <class X>
template <class Rep, class Period, class Predicate>
inline X Deque<X>::popFor(bool front,
                          const std::chrono::duration<Rep, Period> &timeout,
                          Predicate pred) {
  auto begin_time = high_resolution_clock::now();
  for (auto ul = std::unique_lock<std::recursive_mutex>(mtx_); !pred();) {
    if (!deque_.empty()) {
      MYSPACE_DEFER(cond_.notify_one());
      MYSPACE_DEFER(if (front) deque_.pop_front(); else deque_.pop_back(););
      return (front ? deque_.front() : deque_.back());
    } else {
      auto now = high_resolution_clock::now();
      MYSPACE_THROW_IF_EX(concurrency::TimeOut, now - begin_time > timeout);
      cond_.wait_for(ul, now - begin_time,
                     [&]() { return !deque_.empty() || pred(); });
    }
  }
  MYSPACE_THROW_EX(concurrency::TimeOut);
  throw;
}

template <class X> bool Deque<X>::empty() {
  auto ul = std::unique_lock<std::recursive_mutex>(mtx_);
  return deque_.empty();
}

template <class X> size_t Deque<X>::size() {
  auto ul = std::unique_lock<std::recursive_mutex>(mtx_);
  return deque_.size();
}

template <class X> size_t Deque<X>::maxSize() const {
  return std::min(max_size_, deque_.max_size());
}
} // namespace concurrency

MYSPACE_END
