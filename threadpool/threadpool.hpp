
#pragma once

#include "myspace/_/stdafx.hpp"
#include "myspace/critical/critical.hpp"
#include "myspace/mutex/mutex.hpp"
#include "myspace/defer/defer.hpp"

MYSPACE_BEGIN

class ThreadPool {
public:
  ThreadPool(size_t max_threads = std::thread::hardware_concurrency());

  ~ThreadPool();

  template <class Function, class... Arguments>
  auto pushFront(Function &&f, Arguments &&... args)
      -> std::future<typename std::result_of<Function(Arguments...)>::type>;

  template <class Function, class... Arguments>
  auto pushBack(Function &&f, Arguments &&... args)
      -> std::future<typename std::result_of<Function(Arguments...)>::type>;

private:
  template <class Function, class... Arguments>
  auto push(bool putfront, Function &&f, Arguments &&... args)
      -> std::future<typename std::result_of<Function(Arguments...)>::type>;

  void workerProc();

  bool stop_ = false;

  Critical<std::list<std::function<void()>>> jobs_;

  std::deque<std::thread> threads_;
};

inline ThreadPool::ThreadPool(size_t max_threads) {
  if (max_threads == 0)
    max_threads = std::thread::hardware_concurrency();
  if (max_threads == 0)
    max_threads = 1;
  for (size_t i = 0; i < max_threads; ++i) {
    threads_.emplace_back([this]() { this->workerProc(); });
  }
}

template <class Function, class... Arguments>
inline auto ThreadPool::pushFront(Function &&f, Arguments &&... args)
    -> std::future<typename std::result_of<Function(Arguments...)>::type> {
  return std::move(
      push(true, std::forward<Function>(f), std::forward<Arguments>(args)...));
}

template <class Function, class... Arguments>
inline auto ThreadPool::pushBack(Function &&f, Arguments &&... args)
    -> std::future<typename std::result_of<Function(Arguments...)>::type> {
  return std::move(
      push(false, std::forward<Function>(f), std::forward<Arguments>(args)...));
}

inline ThreadPool::~ThreadPool() {
  stop_ = true;
  jobs_.notify_all();
  for (auto &thr : threads_)
    thr.join();
}

template <class Function, class... Arguments>
inline auto ThreadPool::push(bool putfront, Function &&f, Arguments &&... args)
    -> std::future<typename std::result_of<Function(Arguments...)>::type> {
  using return_t = typename std::result_of<Function(Arguments...)>::type;
  auto job = newShared<std::packaged_task<return_t()>>(
      std::bind(std::forward<Function>(f), std::forward<Arguments>(args)...));
  auto ret = job->get_future();
  MYSPACE_IF_LOCK(jobs_) {
    if (putfront) {
      jobs_.emplace_front([job]() { (*job)(); });
    } else {
      jobs_.emplace_back([job]() { (*job)(); });
    }
  }
  jobs_.notify_one();
  return std::move(ret);
}

inline void ThreadPool::workerProc() {
  for (;;) {
    std::function<void()> job;
    MYSPACE_IF_LOCK(jobs_) {
      if (jobs_.empty()) {
        if (stop_)
          break;
        jobs_.wait_for(__ul, std::chrono::seconds(1),
                       [this]() { return !jobs_.empty() || stop_; });
        continue;
      }
      job.swap(jobs_.front());
      jobs_.pop_front();
    }
    if (job) {
      try {
        job();
      } catch (...) {
      }
    }
  }
}

MYSPACE_END
