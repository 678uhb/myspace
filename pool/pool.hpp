
#include "myspace/_/stdafx.hpp"

MYSPACE_BEGIN

template <class X, class Creator, class Deleter> class Pool;

class PoolFactory {
  template <class X> class DefaultCreator {
  public:
    X *operator()() const { return new X(); }
  };

public:
  template <class X>
  static auto create(size_t max_count = 0)
      -> std::shared_ptr<Pool<X, DefaultCreator<X>, std::default_delete<X>>>;

  template <class X, class Creator>
  static auto create(size_t max_count, Creator &&creator)
      -> std::shared_ptr<Pool<X, Creator, std::default_delete<X>>>;

  template <class X, class Creator>
  static auto create(size_t max_count, const Creator &creator)
      -> std::shared_ptr<Pool<X, Creator, std::default_delete<X>>>;

  template <class X, class Creator, class Deleter>
  static auto create(size_t max_count, Creator &&creator, Deleter &&deleter)
      -> std::shared_ptr<Pool<X, Creator, Deleter>>;
};

template <class X, class Creator, class Deleter>
class Pool : public std::enable_shared_from_this<Pool<X, Creator, Deleter>> {
  using std::enable_shared_from_this<Pool>::shared_from_this;
  typedef std::unique_ptr<X, Deleter> StorageType;

public:
  std::unique_ptr<X, std::function<void(X *)>> getUnique();

  std::shared_ptr<X> getShared();

private:
  StorageType _getAvaible();

  Pool(size_t max_count, Creator &&creator, Deleter &&deleter);

  void put(X *px);

  StorageType _createItem();

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
inline std::unique_ptr<X, std::function<void(X *)>> Pool<X, C, D>::getUnique() {
  auto pool = shared_from_this();
  std::unique_ptr<X, std::function<void(X *)>> up(
      _getAvaible().release(), [pool](X *px) { pool->put(px); });
  return std::move(up);
}

template <class X, class C, class D>
inline std::shared_ptr<X> Pool<X, C, D>::getShared() {
  auto pool = shared_from_this();
  std::shared_ptr<X> sp(_getAvaible().release(),
                        [pool](X *px) { pool->put(px); });
  return std::move(sp);
}

template <class X, class C, class D>
inline typename Pool<X, C, D>::StorageType Pool<X, C, D>::_getAvaible() {
  Pool::StorageType x(nullptr, deleter_);

  for (auto ul = std::unique_lock<std::mutex>(mtx_);;) {
    if (!avaible_.empty()) {
      x.reset(avaible_.front().release());

      avaible_.pop_front();

      occupied_++;

      break;
    } else if (max_ == 0 || avaible_.size() + occupied_ < max_) {
      occupied_++;

      return std::move(_createItem());
    } else {
      cond_.wait_for(ul, std::chrono::seconds(1));
    }
  }

  return std::move(x);
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

template <class X, class C, class D>
inline typename Pool<X, C, D>::StorageType Pool<X, C, D>::_createItem() {
  return std::move(StorageType(creator_(), deleter_));
}

template <class X>
inline auto PoolFactory::create(size_t max_count) -> std::shared_ptr<
    Pool<X, PoolFactory::DefaultCreator<X>, std::default_delete<X>>> {
  typedef Pool<X, PoolFactory::DefaultCreator<X>, std::default_delete<X>>
      PoolType;

  return std::shared_ptr<PoolType>(new PoolType(
      max_count, PoolFactory::DefaultCreator<X>(), std::default_delete<X>()));
}

template <class X, class Creator>
inline auto PoolFactory::create(size_t max_count, Creator &&creator)
    -> std::shared_ptr<Pool<X, Creator, std::default_delete<X>>> {
  typedef Pool<X, Creator, std::default_delete<X>> PoolType;

  return std::shared_ptr<PoolType>(new PoolType(
      max_count, std::forward<Creator>(creator), std::default_delete<X>()));
}

template <class X, class Creator>
inline auto PoolFactory::create(size_t max_count, const Creator &creator)
    -> std::shared_ptr<Pool<X, Creator, std::default_delete<X>>> {
  typedef Pool<X, Creator, std::default_delete<X>> PoolType;

  return std::shared_ptr<PoolType>(new PoolType(
      max_count, std::move(Creator(creator)), std::default_delete<X>()));
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
