
#pragma once

#include "myspace/config.h"
#include "myspace/critical/critical.h"
#include "myspace/mutex/mutex.h"

myspace_begin

class ThreadPool
{
public:
	ThreadPool(size_t count = thread::hardware_concurrency());

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

	~ThreadPool();

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
