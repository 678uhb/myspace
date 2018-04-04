
#pragma once

#include "myspace/critical/critical.hpp"
#include "myspace/mutex/mutex.hpp"
#include "myspace/myspace_include.h"
#include "myspace/scope/scope.hpp"

MYSPACE_BEGIN

class ThreadPool;
MYSPACE_API extern ThreadPool threadpool;

class ThreadPool {
public:
  ThreadPool(size_t maxThreads = thread::hardware_concurrency());

  template <class ft, class... argst>
  auto pushFront(ft &&f, argst &&... args)
      -> future<typename result_of<ft(argst...)>::type>;

  template <class ft, class... argst>
  auto pushBack(ft &&f, argst &&... args)
      -> future<typename result_of<ft(argst...)>::type>;

  ~ThreadPool();

private:
  template <class ft, class... argst>
  auto push(bool putfront, ft &&f, argst &&... args)
      -> future<typename result_of<ft(argst...)>::type>;

  void workerProc();

  size_t maxThreads_;

  bool stop_ = false;

  Critical<list<function<void()>>> jobs_;

  Critical<unordered_map<thread::id, thread>> threads_;
};

ThreadPool::ThreadPool(size_t maxThreads) : maxThreads_(maxThreads) {
  if (maxThreads_ == 0)
    maxThreads_ = thread::hardware_concurrency();
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

  MYSPACE_FOR_LOCK(threads_) {
    if (threads_.empty())
      return;

    threads_.wait_for(__ul, seconds(1), [this]() { return threads_.empty(); });
  }
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

    MYSPACE_IF_LOCK(threads_) {
      if (threads_.size() < maxThreads_) {
        try {
          auto t = thread([this]() { this->workerProc(); });

          threads_[t.get_id()] = move(t);
        } catch (...) {
        }
      }
    }
  }

  jobs_.notify_one();

  return move(ret);
}

void ThreadPool::workerProc() {
  MYSPACE_DEFER(MYSPACE_IF_LOCK(threads_) {
    auto id = this_thread::get_id();

    auto itr = threads_.find(id);

    if (itr != threads_.end()) {
      auto t = move(itr->second);

      threads_.erase(itr);

      threads_.notify_one();

      t.detach();
    }
  });

  for (bool wait = true; !stop_;) {
    function<void()> job;

    MYSPACE_IF_LOCK(jobs_) {
      if (jobs_.empty()) {
        if (!wait) {
          return;
        }

        wait = false;

        jobs_.wait_for(__ul, seconds(1),
                       [this]() { return !jobs_.empty() || stop_; });

        continue;
      }

      job.swap(jobs_.front());

      jobs_.pop_front();
    }

    wait = true;

    if (job) {
      try {
        job();
      } catch (...) {
      }
    }
  }
}

MYSPACE_END
