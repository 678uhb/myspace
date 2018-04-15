
#pragma once

#include "myspace/_/stdafx.hpp"

#if defined(MYSPACE_LINUX)

#include "myspace/exception/exception.hpp"
#include "myspace/logger/logger.hpp"

MYSPACE_BEGIN

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

  bool i_am_caller_ = true;

  bool caller_is_quit_ = false;

  bool callee_is_quit_ = false;

  std::function<void(Ucontext &)> func_;

  std::unique_ptr<ucontext_t> caller_ =
      std::unique_ptr<ucontext_t>(new ucontext_t);

  std::unique_ptr<ucontext_t> callee_ =
      std::unique_ptr<ucontext_t>(new ucontext_t);

  std::unique_ptr<char[]> stack_buffer_;
};

template <class Function, class... Arguments>
inline Ucontext::Ucontext(Function &&func, Arguments &&... args)
    : stack_buffer_(new char[8196]) {
  MYSPACE_DEV("Ucontext()");

  MYSPACE_THROW_IF(::getcontext(callee_.get()) < 0);

  callee_->uc_stack.ss_sp = stack_buffer_.get();

  callee_->uc_stack.ss_size = 8196;

  callee_->uc_link = caller_.get();

  ::makecontext(callee_.get(),
                reinterpret_cast<void (*)(void)>(&Ucontext::proc), 1, this);

  auto f =
      std::bind(std::forward<Function>(func), std::forward<Arguments>(args)...);

  func_ = [f](Ucontext &x) { f(x); };

  MYSPACE_DEV("Ucontext()!!");
}
inline Ucontext::~Ucontext() {
  MYSPACE_DEV("~Ucontext()");

  caller_is_quit_ = true;

  while (!callee_is_quit_) {
    resume();
  }

  MYSPACE_DEV("~Ucontext()!!");
}
inline Ucontext::Ucontext(Ucontext &&u) {
  MYSPACE_DEV("Ucontext(Ucontext&& u)");

  swap(std::move(u));
}
inline void Ucontext::swap(Ucontext &&f) {
  if (&f != this) {
    func_.swap(f.func_);

    callee_is_quit_ = f.callee_is_quit_;
    f.callee_is_quit_ = true;

    caller_is_quit_ = f.caller_is_quit_;
    f.caller_is_quit_ = true;

    caller_.swap(f.caller_);

    callee_.swap(f.callee_);

    stack_buffer_.swap(f.stack_buffer_);

    i_am_caller_ = f.i_am_caller_;
  }
}

inline Ucontext::operator bool() const {
  if (i_am_caller_)
    return !callee_is_quit_;
  return !caller_is_quit_;
}

inline bool Ucontext::callerIsQuit() const { return caller_is_quit_; }

inline bool Ucontext::calleeIsQuit() const { return callee_is_quit_; }

inline void Ucontext::resume() {
  if (i_am_caller_ && !callee_is_quit_) {
    i_am_caller_ = false;
    swapcontext(caller_.get(), callee_.get());
  } else if (!i_am_caller_) {
    i_am_caller_ = true;
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

  caller->callee_is_quit_ = true;

  caller->resume();
}

MYSPACE_END

#endif
