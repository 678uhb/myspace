
#pragma once

#include "myspace/_/stdafx.hpp"

#if defined(MYSPACE_WINDOWS)
#include "myspace/exception/exception.hpp"
#include "myspace/logger/logger.hpp"

MYSPACE_BEGIN

class Fiber {
public:
  template <class Function, class... Arguments>
  Fiber(Function &&func, Arguments &&... args);

  Fiber(Fiber &&f);

  Fiber(const Fiber &) = delete;

  Fiber &operator=(const Fiber &) = delete;

  void swap(Fiber &&f);

  ~Fiber();

  Fiber &resume();

  operator bool() const;

private:
  bool callerIsQuit() const;

  bool calleeIsQuit() const;
  static void CALLBACK proc(_In_ PVOID lpParameter);

  std::function<void(Fiber &)> func_;

  bool caller_is_fiber_ = false;

  bool caller_is_quit_ = false;

  bool callee_is_quit_ = false;

  LPVOID caller_ = nullptr;

  LPVOID callee_ = nullptr;
};

template <class Function, class... Arguments>
inline Fiber::Fiber(Function &&func, Arguments &&... args) {
  caller_ = ::ConvertThreadToFiberEx(this, FIBER_FLAG_FLOAT_SWITCH);

  if (!caller_) {
    caller_is_fiber_ = true;

    caller_ = GetCurrentFiber();
  }

  MYSPACE_THROW_IF(!caller_);

  auto f =
      std::bind(std::forward<Function>(func), std::forward<Arguments>(args)...);

  func_ = [f](Fiber &x) { f(x); };

  callee_ =
      ::CreateFiberEx(1024, 1024, FIBER_FLAG_FLOAT_SWITCH, &Fiber::proc, this);

  MYSPACE_THROW_IF(!callee_);
}

inline Fiber::Fiber(Fiber &&f) { swap(std::move(f)); }

inline void Fiber::swap(Fiber &&f) {
  if (&f != this) {
    func_.swap(f.func_);

    callee_is_quit_ = f.callee_is_quit_;
    f.callee_is_quit_ = true;

    caller_is_quit_ = f.caller_is_quit_;
    f.caller_is_quit_ = true;

    caller_ = f.caller_;
    f.caller_ = nullptr;

    callee_ = f.callee_;
    f.callee_ = nullptr;

    caller_is_fiber_ = f.caller_is_fiber_;
    f.caller_is_fiber_ = true;
  }
}

inline Fiber::~Fiber() {
  caller_is_quit_ = true;

  if (callee_) {
    while (!callee_is_quit_) {
      resume();
    }

    ::DeleteFiber(callee_);
  }

  if (!caller_is_fiber_) {
    ::ConvertFiberToThread();
  }
}

inline Fiber &Fiber::resume() {
  auto whoami = ::GetCurrentFiber();

  if (caller_ && caller_ != whoami) {
    ::SwitchToFiber(caller_);
  } else if (!callee_is_quit_ && callee_ && callee_ != whoami) {
    ::SwitchToFiber(callee_);
  }
  return *this;
}
inline Fiber::operator bool() const {
  if (caller_ == GetCurrentFiber()) {
    return !callee_is_quit_;
  }
  return !caller_is_quit_;
}
inline bool Fiber::callerIsQuit() const { return caller_is_quit_; }
inline bool Fiber::calleeIsQuit() const { return callee_is_quit_; }
inline void CALLBACK Fiber::proc(_In_ PVOID lpParameter) {
  Fiber *f = (Fiber *)lpParameter;

  try {
    if (f->func_) {
      (f->func_)(*f);
    }
  } catch (...) {
    MYSPACE_ERROR(Exception::dump());
  }

  f->callee_is_quit_ = true;

  f->resume();
}

MYSPACE_END

#endif
