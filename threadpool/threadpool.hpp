
#pragma once

#include "myspace/myspace_include.h"
#include "myspace/critical/critical.hpp"
#include "myspace/mutex/mutex.hpp"

myspace_begin

class ThreadPool
{
public:

	ThreadPool(size_t count = thread::hardware_concurrency())
	{
		for (size_t x = 0; x < count; ++x)
		{
			_threads.emplace_back([this]()
			{
				while (!_stop)
				{
					function<void()> job;

					if_lock(_jobs)
					{
						if (_jobs.empty()) {
							_jobs.wait_for(__ul, seconds(1), [this]() {
								return !_jobs.empty() || _stop;
							});
							continue;
						}
						job.swap(_jobs.front());
						_jobs.pop_front();
					}

					if (job)
					{
						try
						{
							job();
						}
						catch (...)
						{

						}
					}
				}
			});
		}
	}


	template<class ft, class... argst>
	auto push_front(ft&& f, argst&&... args)
		-> future<typename result_of<ft(argst...)>::type>
	{
		return move(push(true, forward<ft>(f), forward<argst>(args)...));
	}


	template<class ft, class... argst>
	auto push_back(ft&& f, argst&&... args)
		-> future<typename result_of<ft(argst...)>::type>
	{
		return move(push(false, forward<ft>(f), forward<argst>(args)...));
	}

	~ThreadPool()
	{
		_stop = true;

		_jobs.notify_all();

		for (auto& t : _threads)
		{
			if (t.joinable())
				t.join();
		}
	}

private:

	template<class ft, class... argst>
	auto push(bool putfront, ft&& f, argst&&... args)
		-> future<typename result_of<ft(argst...)>::type>
	{

		using return_t = typename result_of<ft(argst...)>::type;

		auto job = make_shared<packaged_task<return_t()>>(bind(forward<ft>(f), forward<argst>(args)...));

		auto ret = job->get_future();

		if_lock(_jobs) 
		{
			if (putfront)
				_jobs.emplace_front([job]() { (*job)(); });
			else
				_jobs.emplace_back([job]() { (*job)(); });
		}

		_jobs.notify_one();

		return move(ret);
	}

	bool								_stop = false;
	Critical<list<function<void()>>>	_jobs;
	list<thread>						_threads;
};


myspace_end
