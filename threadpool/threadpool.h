
#pragma once

#include "myspace/config.h"
#include "myspace/critical/critical.h"

my_space_begin

class ThreadPool
{
public:
	ThreadPool(size_t count = thread::hardware_concurrency());

	~ThreadPool();

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

private:

	template<class ft, class... argst>
	auto push(bool putfront, ft&& f, argst&&... args)
		-> future<typename result_of<ft(argst...)>::type>
	{

		using return_t = typename result_of<ft(argst...)>::type;

		auto job = make_shared<packaged_task<return_t()>>(bind(forward<ft>(f), forward<argst>(args)...));

		auto ret = job->get_future();

		if_lock(_jobs._mtx) 
		{
			if (putfront)
				_jobs._hold.emplace_front([job]() { (*job)(); });
			else
				_jobs._hold.emplace_back([job]() { (*job)(); });
		}

		_jobs._cond.notify_one();

		return move(ret);
	}

	bool								_stop = false;
	Critial<list<function<void()>>>		_jobs;
	list<thread>						_threads;
};


my_space_end