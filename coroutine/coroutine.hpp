
#pragma once

#include "myspace/_/include.hpp"
#include "myspace/exception/exception.hpp"
#include "myspace/logger/logger.hpp"

MYSPACE_BEGIN

#ifdef MYSPACE_WINDOWS

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

  function<void(Fiber &)> func_;

  bool callerIsFiber_ = false;

  bool callerIsQuit_ = false;

  bool calleeIsQuit_ = false;

  LPVOID caller_ = nullptr;

  LPVOID callee_ = nullptr;
};

#endif

#ifdef MYSPACE_LINUX

class Ucontext {
public:
  template <class Function, class... Arguments>
  Ucontext(Function &&func, Arguments &&... args);

  ~Ucontext();

  Ucontext(Ucontext &&u);

  Ucontext(const Ucontext &) = delete;

  Ucontext operator=(const Ucontext &) = delete;

  void swap(Ucontext &&f);

  operator bool() const;

  void resume();

private:
  bool callerIsQuit() const;

  bool calleeIsQuit() const;

  static void proc(void *arg);

  bool iAmCaller_ = true;

  bool callerIsQuit_ = false;

  bool calleeIsQuit_ = false;

  function<void(Ucontext &)> func_;

  unique_ptr<ucontext_t> caller_ = unique_ptr<ucontext_t>(new ucontext_t);

  unique_ptr<ucontext_t> callee_ = unique_ptr<ucontext_t>(new ucontext_t);

  unique_ptr<char[]> stackBuffer_;
};

#endif

#ifdef MYSPACE_WINDOWS
typedef Fiber Coroutine;
#endif

#ifdef MYSPACE_LINUX
typedef Ucontext Coroutine;
#endif

#ifdef MYSPACE_WINDOWS

template <class Function, class... Arguments>
inline Fiber::Fiber(Function &&func, Arguments &&... args) {
  caller_ = ::ConvertThreadToFiberEx(this, FIBER_FLAG_FLOAT_SWITCH);

  if (!caller_) {
    callerIsFiber_ = true;

    caller_ = GetCurrentFiber();
  }

  MYSPACE_IF_THROW(!caller_);

  auto f = bind(forward<Function>(func), forward<Arguments>(args)...);

  func_ = [f](Fiber &x) { f(x); };

  callee_ =
      ::CreateFiberEx(1024, 1024, FIBER_FLAG_FLOAT_SWITCH, &Fiber::proc, this);

  MYSPACE_IF_THROW(!callee_);
}

inline Fiber::Fiber(Fiber &&f) { swap(move(f)); }

inline void Fiber::swap(Fiber &&f) {
  if (&f != this) {
    func_.swap(f.func_);

    calleeIsQuit_ = f.calleeIsQuit_;
    f.calleeIsQuit_ = true;

    callerIsQuit_ = f.callerIsQuit_;
    f.callerIsQuit_ = true;

    caller_ = f.caller_;
    f.caller_ = nullptr;

    callee_ = f.callee_;
    f.callee_ = nullptr;

    callerIsFiber_ = f.callerIsFiber_;
    f.callerIsFiber_ = true;
  }
}

inline Fiber::~Fiber() {
  callerIsQuit_ = true;

  if (callee_) {
    while (!calleeIsQuit_) {
      resume();
    }

    ::DeleteFiber(callee_);
  }

  if (!callerIsFiber_) {
    ::ConvertFiberToThread();
  }
}

inline Fiber &Fiber::resume() {
  auto whoami = ::GetCurrentFiber();

  if (caller_ && caller_ != whoami) {
    ::SwitchToFiber(caller_);
  } else if (!calleeIsQuit_ && callee_ && callee_ != whoami) {
    ::SwitchToFiber(callee_);
  }
  return *this;
}
inline Fiber::operator bool() const {
  if (caller_ == GetCurrentFiber()) {
    return !calleeIsQuit_;
  }
  return !callerIsQuit_;
}
inline bool Fiber::callerIsQuit() const { return callerIsQuit_; }
inline bool Fiber::calleeIsQuit() const { return calleeIsQuit_; }
inline void CALLBACK Fiber::proc(_In_ PVOID lpParameter) {
  Fiber *f = (Fiber *)lpParameter;

  try {
    if (f->func_) {
      (f->func_)(*f);
    }
  } catch (...) {
    MYSPACE_ERROR(Exception::dump());
  }

  f->calleeIsQuit_ = true;

  f->resume();
}

#endif

#ifdef MYSPACE_LINUX

template <class Function, class... Arguments>
inline Ucontext::Ucontext(Function &&func, Arguments &&... args)
    : stackBuffer_(new char[8196]) {
  MYSPACE_DEV("Ucontext()");

  MYSPACE_IF_THROW(::getcontext(callee_.get()) < 0);

  callee_->uc_stack.ss_sp = stackBuffer_.get();

  callee_->uc_stack.ss_size = 8196;

  callee_->uc_link = caller_.get();

  ::makecontext(callee_.get(),
                reinterpret_cast<void (*)(void)>(&Ucontext::proc), 1, this);

  auto f = bind(forward<Function>(func), forward<Arguments>(args)...);

  func_ = [f](Ucontext &x) { f(x); };

  MYSPACE_DEV("Ucontext()!!");
}
inline Ucontext::~Ucontext() {
  MYSPACE_DEV("~Ucontext()");

  callerIsQuit_ = true;

  while (!calleeIsQuit_) {
    resume();
  }

  MYSPACE_DEV("~Ucontext()!!");
}
inline Ucontext::Ucontext(Ucontext &&u) {
  MYSPACE_DEV("Ucontext(Ucontext&& u)");

  swap(move(u));
}
inline void Ucontext::swap(Ucontext &&f) {
  if (&f != this) {
    func_.swap(f.func_);

    calleeIsQuit_ = f.calleeIsQuit_;
    f.calleeIsQuit_ = true;

    callerIsQuit_ = f.callerIsQuit_;
    f.callerIsQuit_ = true;

    caller_.swap(f.caller_);

    callee_.swap(f.callee_);

    stackBuffer_.swap(f.stackBuffer_);

    iAmCaller_ = f.iAmCaller_;
  }
}

inline Ucontext::operator bool() const {
  if (iAmCaller_)
    return !calleeIsQuit_;
  return !callerIsQuit_;
}

inline bool Ucontext::callerIsQuit() const { return callerIsQuit_; }

inline bool Ucontext::calleeIsQuit() const { return calleeIsQuit_; }

inline void Ucontext::resume() {
  if (iAmCaller_ && !calleeIsQuit_) {
    iAmCaller_ = false;
    swapcontext(caller_.get(), callee_.get());
  } else if (!iAmCaller_) {
    iAmCaller_ = true;
    swapcontext(callee_.get(), caller_.get());
  }
}
inline void Ucontext::proc(void *arg) {
  auto caller = (Ucontext *)arg;

  try {
    if (caller && caller->func_) {
      (caller->func_)(*caller);
    }
  } catch (...) {
  }

  caller->calleeIsQuit_ = true;

  caller->resume();
}

#endif

MYSPACE_END
