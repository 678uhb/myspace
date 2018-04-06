
#pragma once

#include "myspace/_/include.hpp"
#include "myspace/annoymous/annoymous.hpp"

MYSPACE_BEGIN

class Scope {
public:
  template <class f_t, class... a_t> Scope(f_t &&f, a_t &&... args);

  ~Scope();

  void dismiss();

private:
  function<void()> defered_ = nullptr;
};

template <class f_t, class... a_t>
inline Scope::Scope(f_t &&f, a_t &&... args)
    : defered_(bind(forward<f_t &&>(f), forward<a_t &&>(args)...)) {}

inline Scope::~Scope() {
  if (defered_) {
    try {
      defered_();
    } catch (...) {
    };
  }
}

inline void Scope::dismiss() { defered_ = nullptr; }

#define MYSPACE_DEFER(f) MYSPACE_ANNOYMOUS(Scope)([&]() { f; })

MYSPACE_END