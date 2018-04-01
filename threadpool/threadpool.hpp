
#pragma once

#include "myspace/myspace_include.h"
#include "myspace/critical/critical.hpp"
#include "myspace/mutex/mutex.hpp"

MYSPACE_BEGIN

class ThreadPool
{
public:

	ThreadPool(size_t maxThreads = thread::hardware_concurrency())
		:_maxThreads(maxThreads)
	{
		if(_maxThreads == 0)
			_maxThreads = thread::hardware_concurrency();
	}

	template<class ft, class... argst>
	auto pushFront(ft&& f, argst&&... args)
		-> future<typename result_of<ft(argst...)>::type>
	{
		return move(push(true, forward<ft>(f), forward<argst>(args)...));
	}


	template<class ft, class... argst>
	auto pushBack(ft&& f, argst&&... args)
		-> future<typename result_of<ft(argst...)>::type>
	{
		return move(push(false, forward<ft>(f), forward<argst>(args)...));
	}

	~ThreadPool()
	{
		_stop = true;

		_jobs.notify_all();

		MYSPACE_FOR_LOCK(_threads)
		{
			if (_threads == 0)
				return;
			
			_threads.wait_for(__ul, seconds(1), [this]()
			{
				return _threads == 0;
			});
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

		MYSPACE_IF_LOCK(_jobs) 
		{
			if (putfront)
			{
				_jobs.emplace_front([job]() { (*job)(); });
			}
			else
			{
				_jobs.emplace_back([job]() { (*job)(); });
			}

			MYSPACE_IF_LOCK(_threads)
			{
				if(_threads < _maxThreads)
				{
					++_threads;

					thread([this](){ this->workerProc(); }).detach();
				}
			}
		}

		_jobs.notify_one();
		
		return move(ret);
	}

	void workerProc()
	{
		for (bool wait = true; !_stop ; )
		{
			function<void()> job;

			MYSPACE_IF_LOCK(_jobs)
			{
				if (_jobs.empty())
				{
					if(!wait)
					{
						MYSPACE_IF_LOCK(_threads)
						{
							--_threads;
						}

						_threads.notify_one();

						return;
					}

					wait = false;

					_jobs.wait_for(__ul, seconds(1), [this]() 
					{
						return !_jobs.empty() || _stop;
					});

					continue;
				}
				
				job.swap(_jobs.front());

				_jobs.pop_front();
			}

			wait = true;

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
	}

	size_t								_maxThreads;

	bool								_stop = false;

	Critical<list<function<void()>>>	_jobs;

	Critical<size_t>					_threads = 0;
};


MYSPACE_END
