
#include "myspace/threadpool/threadpool.h"
#include "myspace/mutex/mutex.h"

my_space_begin


ThreadPool::ThreadPool(size_t count)
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

ThreadPool::~ThreadPool()
{
	_stop = true;

	_jobs.notify_all();

	for (auto& t : _threads) 
	{
		if (t.joinable())
			t.join();
	}
}


my_space_end