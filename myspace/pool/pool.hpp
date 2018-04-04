
#include "myspace/myspace_include.h"

MYSPACE_BEGIN

template <class X, class Creator, class Deleter>
class Pool : public enable_shared_from_this<Pool<X, Creator, Deleter>> {
  using enable_shared_from_this<Pool>::shared_from_this;
  typedef unique_ptr<X, Deleter> StorageType;
  friend class PoolFactory;

public:
  unique_ptr<X, std::function<void(X *)>> getUnique();

  shared_ptr<X> getShared();

private:
  StorageType _getAvaible();

  Pool(size_t max, Creator &&creator, Deleter &&deleter);

  void put(X *px);

  StorageType _createItem();

private:
  size_t max_ = 0;
  size_t occupied_ = 0;
  mutex mtx_;
  condition_variable cond_;
  deque<StorageType> avaible_;
  Creator creator_;
  Deleter deleter_;
};

class PoolFactory {
  template <class X> class DefaultCreator {
  public:
    X *operator()() const { return new X(); }
  };

public:
  template <class X>
  static auto create(size_t max = 0)
      -> shared_ptr<Pool<X, DefaultCreator<X>, default_delete<X>>>;

  template <class X, class Creator>
  static auto create(size_t max, Creator &&creator)
      -> shared_ptr<Pool<X, Creator, default_delete<X>>>;

  template <class X, class Creator>
  static auto create(size_t max, const Creator &creator)
      -> shared_ptr<Pool<X, Creator, default_delete<X>>>;

  template <class X, class Creator, class Deleter>
  static auto create(size_t max, Creator &&creator, Deleter &&deleter)
      -> shared_ptr<Pool<X, Creator, Deleter>>;
};

template <class X, class C, class D>
inline unique_ptr<X, std::function<void(X *)>> Pool<X, C, D>::getUnique() {
  unique_ptr<X, std::function<void(X *)>> up(
      _getAvaible().release(),
      [pool = shared_from_this()](X *px) { pool->put(px); });

  return move(up);
}

template <class X, class C, class D>
inline shared_ptr<X> Pool<X, C, D>::getShared() {
  shared_ptr<X> sp(_getAvaible().release(),
                   [pool = shared_from_this()](X *px) { pool->put(px); });

  return move(sp);
}

template <class X, class C, class D>
inline typename Pool<X, C, D>::StorageType Pool<X, C, D>::_getAvaible() {
  Pool::StorageType x(nullptr, deleter_);

  for (auto ul = unique_lock<mutex>(mtx_);;) {
    if (!avaible_.empty()) {
      x.reset(avaible_.front().release());

      avaible_.pop_front();

      occupied_++;

      break;
    } else if (max_ == 0 || avaible_.size() + occupied_ < max_) {
      occupied_++;

      return move(_createItem());
    } else {
      cond_.wait_for(ul, seconds(1));
    }
  }

  return move(x);
}

template <class X, class C, class D>
inline Pool<X, C, D>::Pool(size_t max, C &&creator, D &&deleter)
    : max_(max), creator_(forward<C>(creator)), deleter_(forward<D>(deleter)) {}

template <class X, class C, class D> inline void Pool<X, C, D>::put(X *px) {
  StorageType x(px, deleter_);
  auto ul = unique_lock<mutex>(mtx_);
  avaible_.push_back(move(x));
  --occupied_;
  ul.unlock();
  cond_.notify_one();
}

template <class X, class C, class D>
inline typename Pool<X, C, D>::StorageType Pool<X, C, D>::_createItem() {
  return move(StorageType(creator_(), deleter_));
}

template <class X>
inline auto PoolFactory::create(size_t max)
    -> shared_ptr<Pool<X, PoolFactory::DefaultCreator<X>, default_delete<X>>> {
  typedef Pool<X, PoolFactory::DefaultCreator<X>, default_delete<X>> PoolType;

  return shared_ptr<PoolType>(
      new PoolType(max, PoolFactory::DefaultCreator<X>(), default_delete<X>()));
}

template <class X, class Creator>
inline auto PoolFactory::create(size_t max, Creator &&creator)
    -> shared_ptr<Pool<X, Creator, default_delete<X>>> {
  typedef Pool<X, Creator, default_delete<X>> PoolType;

  return shared_ptr<PoolType>(
      new PoolType(max, forward<Creator>(creator), default_delete<X>()));
}

template <class X, class Creator>
inline auto PoolFactory::create(size_t max, const Creator &creator)
    -> shared_ptr<Pool<X, Creator, default_delete<X>>> {
  typedef Pool<X, Creator, default_delete<X>> PoolType;

  return shared_ptr<PoolType>(
      new PoolType(max, move(Creator(creator)), default_delete<X>()));
}

template <class X, class Creator, class Deleter>
inline auto PoolFactory::create(size_t max, Creator &&creator,
                                Deleter &&deleter)
    -> shared_ptr<Pool<X, Creator, Deleter>> {
  typedef Pool<X, Creator, Deleter> PoolType;

  return shared_ptr<PoolType>(
      new PoolType(max, forward<Creator>(creator), forward<Deleter>(deleter)));
}

MYSPACE_END
