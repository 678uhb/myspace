
#pragma once

#include "myspace/_/include.hpp"
#include "myspace/critical/critical.hpp"
#include "myspace/mutex/mutex.hpp"
#include "myspace/scope/scope.hpp"

MYSPACE_BEGIN

class ThreadPool {
public:
  ThreadPool(size_t maxThreads = thread::hardware_concurrency());

  ~ThreadPool();

  template <class ft, class... argst>
  auto pushFront(ft &&f, argst &&... args)
      -> future<typename result_of<ft(argst...)>::type>;

  template <class ft, class... argst>
  auto pushBack(ft &&f, argst &&... args)
      -> future<typename result_of<ft(argst...)>::type>;

private:
  template <class ft, class... argst>
  auto push(bool putfront, ft &&f, argst &&... args)
      -> future<typename result_of<ft(argst...)>::type>;

  void workerProc();

  bool stop_ = false;

  Critical<list<function<void()>>> jobs_;

  deque<thread> threads_;
};

ThreadPool::ThreadPool(size_t maxThreads) {
  if (maxThreads == 0)
    maxThreads = thread::hardware_concurrency();
  if (maxThreads == 0)
    maxThreads = 1;
  for (size_t i = 0; i < maxThreads; ++i) {
    threads_.emplace_back([this]() { this->workerProc(); });
  }
}

template <class ft, class... argst>
auto ThreadPool::pushFront(ft &&f, argst &&... args)
    -> future<typename result_of<ft(argst...)>::type> {
  return move(push(true, forward<ft>(f), forward<argst>(args)...));
}

template <class ft, class... argst>
auto ThreadPool::pushBack(ft &&f, argst &&... args)
    -> future<typename result_of<ft(argst...)>::type> {
  return move(push(false, forward<ft>(f), forward<argst>(args)...));
}

ThreadPool::~ThreadPool() {
  stop_ = true;
  jobs_.notify_all();
  for (auto &thread : threads_)
    thread.join();
}

template <class ft, class... argst>
auto ThreadPool::push(bool putfront, ft &&f, argst &&... args)
    -> future<typename result_of<ft(argst...)>::type> {
  using return_t = typename result_of<ft(argst...)>::type;
  auto job = make_shared<packaged_task<return_t()>>(
      bind(forward<ft>(f), forward<argst>(args)...));
  auto ret = job->get_future();
  MYSPACE_IF_LOCK(jobs_) {
    if (putfront) {
      jobs_.emplace_front([job]() { (*job)(); });
    } else {
      jobs_.emplace_back([job]() { (*job)(); });
    }
  }
  jobs_.notify_one();
  return move(ret);
}

void ThreadPool::workerProc() {
  for (;;) {
    function<void()> job;
    MYSPACE_IF_LOCK(jobs_) {
      if (jobs_.empty()) {
        if (stop_)
          break;
        jobs_.wait_for(__ul, seconds(1),
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
