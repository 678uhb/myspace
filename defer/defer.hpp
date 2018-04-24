
#pragma once

#include "myspace/_/stdafx.hpp"
#include "myspace/annoymous/annoymous.hpp"
#include "myspace/exception/exception.hpp"
MYSPACE_BEGIN

#define MYSPACE_DEFER(f) MYSPACE_ANNOYMOUS(myspace::Defer)([&]() { f; })

class Defer {
public:
  template <class Function, class... Arguments>
  Defer(Function &&f, Arguments &&... args);

  ~Defer();

  void dismiss();

private:
  std::function<void()> defered_ = nullptr;
};

template <class Function, class... Arguments>
inline Defer::Defer(Function &&f, Arguments &&... args)
    : defered_(std::bind(std::forward<Function &&>(f),
                         std::forward<Arguments &&>(args)...)) {}

inline Defer::~Defer() {
  if (defered_) {
    try {
      defered_();
    } catch (...) {
      MYSPACE_DEV_EXCEPTION();
    };
  }
}

inline void Defer::dismiss() { defered_ = nullptr; }

MYSPACE_END