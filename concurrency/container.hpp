#pragma once
#include "myspace/_/stdafx.hpp"
#include "myspace/concurrency/exception.hpp"
MYSPACE_BEGIN

namespace concurrency {

template <class X, template <class...> class Cont> class Container {
public:
  void notifyAll();

  void notifyOne();

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
  Container(size_t max_size);
  size_t max_size_ = 0;
  std::recursive_mutex mtx_;
  std::condition_variable_any cond_;
  Cont<X> cont_;

  friend class concurrency::Factory;
};

namespace contimpl {
template <class X, template <class...> class C> X pop(bool front, C<X> &c) {
  MYSPACE_DEFER(if (front) c.pop_front(); else c.pop_back(););
  return (front ? c.front() : c.back());
}
template <class X> X pop(bool front, std::set<X> &c) {
  MYSPACE_DEFER(if (front) c.erase(c.begin());
                else c.erase(std::prev(c.end())););
  return (front ? *c.begin() : *c.rbegin());
}
template <class X, template <class...> class C>
void push(bool front, C<X> &c, const X &x) {
  if (front)
    c.push_front(x);
  else
    c.push_back(x);
}
template <class X> void push(bool front, std::set<X> &c, const X &x) {
  c.insert(x);
}
} // namespace contimpl

template <class X, template <class...> class Cont>
template <class Predicate>
inline void Container<X, Cont>::pushFront(const X &x, Predicate pred) {
  push(true, x, pred);
}

template <class X, template <class...> class Cont>
template <class Predicate>
inline void Container<X, Cont>::pushBack(const X &x, Predicate pred) {
  pop(false, x, pred);
}

template <class X, template <class...> class Cont>
template <class Predicate>
inline X Container<X, Cont>::popFront(Predicate pred) {
  return pop(true, pred);
}

template <class X, template <class...> class Cont>
template <class Predicate>
inline X Container<X, Cont>::popBack(Predicate pred) {
  return pop(false, pred);
}

template <class X, template <class...> class Cont>
inline X Container<X, Cont>::popFront() {
  return pop(true);
}

template <class X, template <class...> class Cont>
template <class Rep, class Period>
inline X Container<X, Cont>::popFrontFor(
    const std::chrono::duration<Rep, Period> &timeout) noexcept(false) {
  return popFor(true, timeout);
}

template <class X, template <class...> class Cont>
template <class Rep, class Period, class Predicate>
inline X Container<X, Cont>::popFrontFor(
    const std::chrono::duration<Rep, Period> &timeout,
    Predicate pred) noexcept(false) {
  return popFor(true, timeout, pred);
}

template <class X, template <class...> class Cont>
inline X Container<X, Cont>::popBack() {
  return pop(false);
}

template <class X, template <class...> class Cont>
template <class Rep, class Period>
inline X Container<X, Cont>::popBackFor(
    const std::chrono::duration<Rep, Period> &timeout) noexcept(false) {
  return popFor(false, timeout);
}

template <class X, template <class...> class Cont>
template <class Rep, class Period, class Predicate>
inline X Container<X, Cont>::popBackFor(
    const std::chrono::duration<Rep, Period> &timeout,
    Predicate pred) noexcept(false) {
  return popFor(false, timeout, pred);
}

template <class X, template <class...> class Cont>
inline void Container<X, Cont>::pushFront(const X &x) {
  push(true, x);
}

template <class X, template <class...> class Cont>
template <class Rep, class Period>
inline void Container<X, Cont>::pushFrontFor(
    const X &x,
    const std::chrono::duration<Rep, Period> &timeout) noexcept(false) {
  pushFor(true, x, timeout);
}

template <class X, template <class...> class Cont>
template <class Rep, class Period, class Predicate>
inline void Container<X, Cont>::pushFrontFor(
    const X &x, const std::chrono::duration<Rep, Period> &timeout,
    Predicate pred) noexcept(false) {
  pushFor(true, x, timeout, pred);
}

template <class X, template <class...> class Cont>
inline void Container<X, Cont>::pushBack(const X &x) {
  push(false, x);
}

template <class X, template <class...> class Cont>
template <class Rep, class Period>
inline void Container<X, Cont>::pushBackFor(
    const X &x,
    const std::chrono::duration<Rep, Period> &timeout) noexcept(false) {
  pushFor(false, x, timeout);
}

template <class X, template <class...> class Cont>
template <class Rep, class Period, class Predicate>
inline void Container<X, Cont>::pushBackFor(
    const X &x, const std::chrono::duration<Rep, Period> &timeout,
    Predicate pred) noexcept(false) {
  push(false, x, timeout, pred);
}

template <class X, template <class...> class Cont>
inline Container<X, Cont>::Container(size_t max_size) : max_size_(max_size) {}

template <class X, template <class...> class Cont>
inline void Container<X, Cont>::push(bool front, const X &x) {
  for (auto ul = std::unique_lock<std::recursive_mutex>(mtx_);;) {
    if (cont_.size() < maxSize()) {
      contimpl::push(front, cont_, x);
      break;
    } else {
      cond_.wait(ul, [&]() { return cont_.size() < maxSize(); });
    }
  }
  cond_.notify_one();
}

template <class X, template <class...> class Cont>
template <class Predicate>
inline void Container<X, Cont>::push(bool front, const X &x, Predicate pred) {
  for (auto ul = std::unique_lock<std::recursive_mutex>(mtx_); !pred();) {
    if (cont_.size() < maxSize()) {
      MYSPACE_DEFER(cond_.notify_one());
      if (front)
        cont_.push_front(x);
      else
        cont_.push_back(x);
      return;
    } else {
      cond_.wait(ul, [&]() { return cont_.size() < maxSize() || pred(); });
    }
  }
  MYSPACE_THROW_EX(PredicateMeet);
}

template <class X, template <class...> class Cont>
template <class Rep, class Period>
void Container<X, Cont>::pushFor(
    bool front, const X &x, const std::chrono::duration<Rep, Period> &timeout) {
  auto begin_time = std::chrono::high_resolution_clock::now();
  for (auto ul = std::unique_lock<std::recursive_mutex>(mtx_);;) {
    if (cont_.size() < maxSize()) {
      if (front)
        cont_.push_front(x);
      else
        cont_.push_back(x);
      ul.unlock();
      break;
    } else {
      auto now = std::chrono::high_resolution_clock::now();
      MYSPACE_THROW_IF_EX(concurrency::TimeOut, now - begin_time > timeout);
      cond_.wait_for(ul, now - begin_time,
                     [&]() { return cont_.size() < maxSize(); });
    }
  }
  cond_.notify_one();
}
template <class X, template <class...> class Cont>
template <class Rep, class Period, class Predicate>
void Container<X, Cont>::pushFor(
    bool front, const X &x, const std::chrono::duration<Rep, Period> &timeout,
    Predicate pred) {
  auto begin_time = std::chrono::high_resolution_clock::now();
  for (auto ul = std::unique_lock<std::recursive_mutex>(mtx_); !pred();) {
    if (cont_.size() < maxSize()) {
      if (front)
        cont_.push_front(x);
      else
        cont_.push_back(x);
      ul.unlock();
      break;
    } else {
      auto now = std::chrono::high_resolution_clock::now();
      MYSPACE_THROW_IF_EX(concurrency::TimeOut, now - begin_time > timeout);
      cond_.wait_for(ul, now - begin_time,
                     [&]() { return cont_.size() < maxSize() || pred(); });
    }
  }
  cond_.notify_one();
}

template <class X, template <class...> class Cont>
inline X Container<X, Cont>::pop(bool front) {
  MYSPACE_DEFER(cond_.notify_one());
  for (auto ul = std::unique_lock<std::recursive_mutex>(mtx_);;) {
    if (!cont_.empty()) {
      MYSPACE_DEFER(if (front) cont_.pop_front(); else cont_.pop_back(););
      return (front ? cont_.front() : cont_.back());
    } else {
      cond_.wait_for(ul, [&]() { return !cont_.empty(); });
    }
  }
  throw; // not reached
}

template <class X, template <class...> class Cont>
template <class Predicate>
inline X Container<X, Cont>::pop(bool front, Predicate pred) {
  MYSPACE_DEFER(cond_.notify_one());
  for (auto ul = std::unique_lock<std::recursive_mutex>(mtx_); !pred();) {
    if (!cont_.empty()) {
      MYSPACE_DEFER(if (front) cont_.pop_front(); else cont_.pop_back(););
      return (front ? cont_.front() : cont_.back());
    } else {
      cond_.wait(ul, [&]() { return !cont_.empty() || pred(); });
    }
  }
  MYSPACE_THROW_EX(PredicateMeet);
  throw; // not reached
}
template <class X, template <class...> class Cont>
inline void Container<X, Cont>::notifyAll() {
  cond_.notify_all();
}
template <class X, template <class...> class Cont>
inline void Container<X, Cont>::notifyOne() {
  cond_.notify_one();
}

template <class X, template <class...> class Cont>
template <class Rep, class Period>
inline X
Container<X, Cont>::popFor(bool front,
                           const std::chrono::duration<Rep, Period> &timeout) {
  auto begin_time = std::chrono::high_resolution_clock::now();
  for (auto ul = std::unique_lock<std::recursive_mutex>(mtx_);;) {
    if (!cont_.empty()) {
      MYSPACE_DEFER(cond_.notify_one());
      return contimpl::pop(front, cont_);
    } else {
      auto now = std::chrono::high_resolution_clock::now();
      MYSPACE_THROW_IF_EX(concurrency::TimeOut, now - begin_time > timeout);
      cond_.wait_for(ul, now - begin_time, [&]() { return !cont_.empty(); });
    }
  }
  MYSPACE_THROW_EX(concurrency::TimeOut);
  throw;
}
template <class X, template <class...> class Cont>
template <class Rep, class Period, class Predicate>
inline X
Container<X, Cont>::popFor(bool front,
                           const std::chrono::duration<Rep, Period> &timeout,
                           Predicate pred) {
  auto begin_time = std::chrono::high_resolution_clock::now();
  for (auto ul = std::unique_lock<std::recursive_mutex>(mtx_); !pred();) {
    if (!cont_.empty()) {
      MYSPACE_DEFER(cond_.notify_one());
      MYSPACE_DEFER(if (front) cont_.pop_front(); else cont_.pop_back(););
      return (front ? cont_.front() : cont_.back());
    } else {
      auto now = std::chrono::high_resolution_clock::now();
      MYSPACE_THROW_IF_EX(concurrency::TimeOut, now - begin_time > timeout);
      cond_.wait_for(ul, now - begin_time,
                     [&]() { return !cont_.empty() || pred(); });
    }
  }
  MYSPACE_THROW_EX(concurrency::TimeOut);
  throw;
}

template <class X, template <class...> class Cont>
bool Container<X, Cont>::empty() {
  auto ul = std::unique_lock<std::recursive_mutex>(mtx_);
  return cont_.empty();
}

template <class X, template <class...> class Cont>
size_t Container<X, Cont>::size() {
  auto ul = std::unique_lock<std::recursive_mutex>(mtx_);
  return cont_.size();
}

template <class X, template <class...> class Cont>
size_t Container<X, Cont>::maxSize() const {
  return std::min(max_size_, cont_.max_size());
}
} // namespace concurrency

MYSPACE_END
