#pragma once

#include "myspace/_/stdafx.hpp"

MYSPACE_BEGIN

class Any {
  template <class X> using StorageType = typename std::decay<X>::type;

public:
  Any() ;

  Any(const Any &a);

  Any(Any &&a) ;

  template <class X,
            typename std::enable_if<
                !std::is_same<typename std::decay<X>::type, Any>::value,
                int>::type = 0>
  Any(X &&x);

  ~Any();

  Any &operator=(const Any &a);

  Any &operator=(Any &&a);

  template <class X,
            typename std::enable_if<
                !std::is_same<typename std::decay<X>::type, Any>::value,
                int>::type = 0>
  Any &operator=(const X &x);

  operator bool() const;

  bool hasValue() const;

  template <class X> bool is() const;

  template <class X> StorageType<X> &as();

  template <class X> operator X();

private:
  struct Base {
    virtual ~Base() {}
    virtual Base *clone() const = 0;
  };

  template <typename X> struct Derived : Base {
    template <typename T> Derived(T &&x) : value_(std::forward<T>(x)) {}

    Base *clone() const { return new Derived<X>(value_); }

    X value_;
  };

  Base *clone() const {
    if (ptr_)
      return ptr_->clone();
    return nullptr;
  }

  Base *ptr_ = nullptr;
};

inline Any::Any()  : ptr_(nullptr) {}

inline Any::Any(const Any &a) : ptr_(a.clone()) {}

inline Any::Any(Any &&a)  : ptr_(a.ptr_) { a.ptr_ = nullptr; }

template <class X, typename std::enable_if<
                       !std::is_same<typename std::decay<X>::type, Any>::value,
                       int>::type>
inline Any::Any(X &&x) {
  auto p = new Derived<Any::StorageType<X>>(std::forward<X>(x));
  ptr_ = p;
}

inline Any &Any::operator=(const Any &a) {
  if (ptr_ == a.ptr_)
    return *this;
  auto old = ptr_;
  ptr_ = a.ptr_->clone();
  if (old)
    delete old;
  return *this;
}

inline Any &Any::operator=(Any &&a) {
  if (ptr_ == a.ptr_)
    return *this;
  ptr_ = a.ptr_;
  a.ptr_ = nullptr;
  return *this;
}

template <class X, typename std::enable_if<
                       !std::is_same<typename std::decay<X>::type, Any>::value,
                       int>::type>
inline Any &Any::operator=(const X &x) {
  this->operator=(std::move(Any(x)));
}

inline Any::~Any() { delete ptr_; }

inline Any::operator bool() const { return hasValue(); }

inline bool Any::hasValue() const { return !!ptr_; }

template <class X> inline bool Any::is() const {
  typedef Any::StorageType<X> T;
  return !!dynamic_cast<Derived<T> *>(ptr_);
}

template <class X> inline Any::StorageType<X> &Any::as() {
  typedef Any::StorageType<X> T;
  auto derived = dynamic_cast<Derived<T> *>(ptr_);
  if (!derived)
    throw std::bad_cast();
  return derived->value_;
}

template <class X> inline Any::operator X() {
  return as<Any::StorageType<X>>();
}

MYSPACE_END