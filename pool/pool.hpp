
#include "myspace/_/stdafx.hpp"
#include "myspace/exception/exception.hpp"

MYSPACE_BEGIN

template <class X, class Creator, class Deleter> class Pool;

class PoolFactory {
public:
  template <class X, class Creator, class Deleter>
  static auto create(size_t max_count, Creator &&creator, Deleter &&deleter)
      -> std::shared_ptr<Pool<X, Creator, Deleter>>;
};

template <class X, class Creator, class Deleter>
class Pool : public std::enable_shared_from_this<Pool<X, Creator, Deleter>> {
  using std::enable_shared_from_this<Pool>::shared_from_this;
  typedef std::unique_ptr<X, Deleter> StorageType;

public:
  // these function may throw exception, due to user defined creater/deleter
  // deleter may not to throw exception
  // c type HANDLE(void*) , better to set X to void, which return value is
  // std::shared_ptr<void>, to avoid trouble
  // all the impliments like std::condition_variable
  //
  std::shared_ptr<X> tryGet();
  //
  std::shared_ptr<X> get();
  //
  template <class Predicate> std::shared_ptr<X> get(Predicate pred);
  //
  template <class Rep, class Period>
  std::shared_ptr<X> getFor(const std::chrono::duration<Rep, Period> &timeout);
  //
  template <class Rep, class Period, class Predicate>
  std::shared_ptr<X> getFor(const std::chrono::duration<Rep, Period> &timeout,
                            Predicate pred);

private:
  Pool(const Pool &) = delete;
  Pool &operator=(const Pool &) = delete;

  Pool(size_t max_count, Creator &&creator, Deleter &&deleter);

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

  friend class PoolFactory;
};
template <class X, class C, class D>
inline std::shared_ptr<X> Pool<X, C, D>::tryGetUnlocked() {
  if (!avaible_.empty()) {
    auto pool = shared_from_this();
    std::shared_ptr<X> sp(avaible_.front().release(),
                          [pool](X *x) { pool->put(x); });
    avaible_.pop_front();
    occupied_++;
    return sp;
  }

  if (max_ == 0 || avaible_.size() + occupied_ < max_) {
    auto x = creator_();
    if (x) {
      occupied_++;
      auto pool = shared_from_this();
      return std::shared_ptr<X>(x, [pool](X *x) { pool->put(x); });
    }
  }
  return nullptr;
}

template <class X, class C, class D>
inline typename std::shared_ptr<X> Pool<X, C, D>::tryGet() {
  auto ul = std::unique_lock<std::mutex>(mtx_);
  return tryGetUnlocked();
}

template <class X, class C, class D>
template <class Predicate>
inline typename std::shared_ptr<X> Pool<X, C, D>::get(Predicate pred) {
  auto ul = std::unique_lock<std::mutex>(mtx_);
  while (!pred()) {
    auto sp = tryGetUnlocked();
    if (sp)
      return sp;
    cond_.wait(ul);
  }
  return tryGetUnlocked();
}

template <class X, class C, class D>
inline typename std::shared_ptr<X> Pool<X, C, D>::get() {
  auto ul = std::unique_lock<std::mutex>(mtx_);
  auto sp = tryGetUnlocked();
  if (sp)
    return sp;
  cond_.wait(ul);
  return tryGetUnlocked();
}

template <class X, class C, class D>
template <class Rep, class Period>
inline typename std::shared_ptr<X>
Pool<X, C, D>::getFor(const std::chrono::duration<Rep, Period> &timeout) {
  auto ul = std::unique_lock<std::mutex>(mtx_);
  auto sp = tryGetUnlocked();
  if (sp)
    return sp;
  cond_.wait_for(timeout);
  return tryGetUnlocked();
}

template <class X, class C, class D>
template <class Rep, class Period, class Predicate>
inline typename std::shared_ptr<X>
Pool<X, C, D>::getFor(const std::chrono::duration<Rep, Period> &timeout,
                      Predicate pred) {
  auto ul = std::unique_lock<std::mutex>(mtx_);
  auto begin_time = std::chrono::high_resolution_clock::now();
  while (!pred()) {
    auto sp = tryGetUnlocked();
    if (sp)
      return sp;
    auto cost_time = std::chrono::high_resolution_clock::now() - begin_time;
    if (cost_time >= timeout) {
      break;
    }
    cond_.wait_for(ul, timeout - cost_time);
  }
  return tryGetUnlocked();
}

template <class X, class C, class D>
inline Pool<X, C, D>::Pool(size_t max_count, C &&creator, D &&deleter)
    : max_(max_count), creator_(std::forward<C>(creator)),
      deleter_(std::forward<D>(deleter)) {}

template <class X, class C, class D> inline void Pool<X, C, D>::put(X *px) {
  StorageType x(px, deleter_);
  auto ul = std::unique_lock<std::mutex>(mtx_);
  avaible_.push_back(std::move(x));
  --occupied_;
  ul.unlock();
  cond_.notify_one();
}

template <class X, class Creator, class Deleter>
inline auto PoolFactory::create(size_t max_count, Creator &&creator,
                                Deleter &&deleter)
    -> std::shared_ptr<Pool<X, Creator, Deleter>> {
  typedef Pool<X, Creator, Deleter> PoolType;

  return std::shared_ptr<PoolType>(
      new PoolType(max_count, std::forward<Creator>(creator),
                   std::forward<Deleter>(deleter)));
}

MYSPACE_END
