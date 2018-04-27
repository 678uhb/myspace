#include "myspace/_/stdafx.hpp"
#include "myspace/exception/exception.hpp"

MYSPACE_BEGIN

MYSPACE_EXCEPTION_DEFINE(PoolError, myspace::Exception)

template <class X, class Creator, class Deleter, class Popper, class Recycler>
class Pool;

class PoolFactory {
public:
  // max_count (0, SIZE_MAX], 0 means SIZE_MAX
  // Creator , which create X*, can NOT be null
  // Deleter, which release X*, can be null
  // Popper, which handle pop X* event, can be null
  // Recycler, which handle recycle X* event, can be null
  template <class X, class Creator, class Deleter, class Popper, class Recycler>
  static auto create(size_t max_count, Creator &&creator, Deleter &&deleter,
                     Popper &&popper, Recycler &&recycler)
      -> std::shared_ptr<Pool<X, Creator, Deleter, Popper, Recycler> >;
};

template <class X, class Creator, class Deleter, class Popper, class Recycler>
class Pool : public std::enable_shared_from_this<
                 Pool<X, Creator, Deleter, Popper, Recycler> > {
  using std::enable_shared_from_this<Pool>::shared_from_this;
  typedef std::unique_ptr<X, Deleter> StorageType;

public:
  // these function may throw exception, due to lack of resources
  // or user defined creater/deleter,deleter may not to throw exception
  // c type HANDLE(void*) , better to set X to void,
  // which return value is std::shared_ptr<void>
  // all the impliments like std::condition_variable
  //
  std::shared_ptr<X> tryGet() noexcept(false);
  //
  std::shared_ptr<X> get() noexcept(false);
  //
  template <class Predicate>
  std::shared_ptr<X> get(Predicate pred) noexcept(false);
  //
  template <class Rep, class Period>
  std::shared_ptr<X>
  getFor(const std::chrono::duration<Rep, Period> &timeout) noexcept(false);
  //
  template <class Rep, class Period, class Predicate>
  std::shared_ptr<X> getFor(const std::chrono::duration<Rep, Period> &timeout,
                            Predicate pred) noexcept(false);

private:
  Pool(const Pool &) = delete;
  Pool &operator=(const Pool &) = delete;

  Pool(size_t max_count, Creator &&creator, Deleter &&deleter, Popper &&popper,
       Recycler &&recycler);

  void put(X *x);
  std::shared_ptr<X> tryGetUnlocked();

private:
  size_t max_ = 0;
  size_t occupied_ = 0;
  std::mutex mtx_;
  std::condition_variable cond_;
  std::deque<StorageType> avaible_;
  Creator creator_;
  Deleter deleter_;
  Popper popper_;
  Recycler recycler_;

  friend class PoolFactory;
};
namespace pool_detail {
template <class Popper, class X> inline void call(Popper &pop, X *sp) {
  if (pop)
    pop(sp);
}
template <class X> inline void call(std::nullptr_t &, X *) {}
}
template <class X, class C, class D, class P, class R>
inline std::shared_ptr<X> Pool<X, C, D, P, R>::tryGetUnlocked() {
  auto pool = shared_from_this();
  auto put = [pool](X *x) { pool->put(x); };
  std::shared_ptr<X> sp;
  if (!avaible_.empty()) {
    sp.reset(avaible_.front().release(), put);
    avaible_.pop_front();
  }
  if (!sp) {
    if (avaible_.size() + occupied_ < max_) {
      auto x = creator_();
      if (x)
        sp.reset(x, put);
    }
  }
  MYSPACE_THROW_IF_EX(PoolError, !sp);
  ++occupied_;
  pool_detail::call(popper_, sp.get());
  return sp;
}

template <class X, class C, class D, class P, class R>
inline typename std::shared_ptr<X>
Pool<X, C, D, P, R>::tryGet() noexcept(false) {
  auto ul = std::unique_lock<std::mutex>(mtx_);
  return tryGetUnlocked();
}

template <class X, class C, class D, class P, class R>
template <class Predicate>
inline typename std::shared_ptr<X>
Pool<X, C, D, P, R>::get(Predicate pred) noexcept(false) {
  auto ul = std::unique_lock<std::mutex>(mtx_);
  while (!pred()) {
    try {
      return tryGetUnlocked();
    }
    catch (...) {
      cond_.wait(ul);
    }
  }
  MYSPACE_THROW_EX(PoolError);
  return nullptr; // not reached
}

template <class X, class C, class D, class P, class R>
inline typename std::shared_ptr<X> Pool<X, C, D, P, R>::get() noexcept(false) {
  auto ul = std::unique_lock<std::mutex>(mtx_);
  try {
    return tryGetUnlocked();
  }
  catch (...) {
  }
  cond_.wait(ul);
  return tryGetUnlocked();
}

template <class X, class C, class D, class P, class R>
template <class Rep, class Period>
inline typename std::shared_ptr<X> Pool<X, C, D, P, R>::getFor(
    const std::chrono::duration<Rep, Period> &timeout) noexcept(false) {
  auto ul = std::unique_lock<std::mutex>(mtx_);
  try {
    return tryGetUnlocked();
  }
  catch (...) {
  }
  cond_.wait_for(ul, timeout);
  return tryGetUnlocked();
}

template <class X, class C, class D, class P, class R>
template <class Rep, class Period, class Predicate>
inline typename std::shared_ptr<X>
Pool<X, C, D, P, R>::getFor(const std::chrono::duration<Rep, Period> &timeout,
                            Predicate pred) noexcept(false) {
  auto ul = std::unique_lock<std::mutex>(mtx_);
  auto begin_time = std::chrono::high_resolution_clock::now();
  while (!pred()) {
    try {
      return tryGetUnlocked();
    }
    catch (...) {
    }
    auto cost_time = std::chrono::high_resolution_clock::now() - begin_time;
    if (cost_time >= timeout) {
      break;
    }
    cond_.wait_for(ul, timeout - cost_time);
  }
  MYSPACE_THROW_EX(PoolError);
  return nullptr; // not reached
}

template <class X, class C, class D, class P, class R>
inline Pool<X, C, D, P, R>::Pool(size_t max_count, C &&c, D &&d, P &&p, R &&r)
    : max_(max_count), creator_(std::forward<C>(c)),
      deleter_(std::forward<D>(d)), popper_(std::forward<P>(p)),
      recycler_(std::forward<R>(r)) {}

template <class X, class C, class D, class P, class R>
inline void Pool<X, C, D, P, R>::put(X *px) {
  auto ul = std::unique_lock<std::mutex>(mtx_);
  --occupied_;
  pool_detail::call(recycler_, px);
  StorageType x(px, deleter_);
  avaible_.push_back(std::move(x));
  ul.unlock();
  cond_.notify_one();
}

template <class X, class Creator, class Deleter, class Popper, class Recycler>
inline auto PoolFactory::create(size_t max_count, Creator &&creator,
                                Deleter &&deleter, Popper &&popper,
                                Recycler &&recycler)
    -> std::shared_ptr<Pool<X, Creator, Deleter, Popper, Recycler> > {
  typedef Pool<X, Creator, Deleter, Popper, Recycler> PoolType;

  return std::shared_ptr<PoolType>(new PoolType(
      (max_count == 0 ? SIZE_MAX : max_count), std::forward<Creator>(creator),
      std::forward<Deleter>(deleter), std::forward<Popper>(popper),
      std::forward<Recycler>(recycler)));
}

MYSPACE_END
