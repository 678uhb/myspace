
#include "myspace/config.hpp"

MYSPACE_BEGIN

template<class X>
class Pool: public enable_shared_from_this<Pool<X>>
{
	using enable_shared_from_this<Pool<X>>::shared_from_this;
public:

	static shared_ptr<Pool<X>> create(size_t max = 0,
		function<unique_ptr<X>()> f = []() { return new_unique<X>(); })
	{
		return shared_ptr<Pool<X>>(new Pool<X>(f, max));
	}

	auto getUnique()
	{
		auto x = _getAvaible();

		unique_ptr<X, std::function<void(X*)>> up(x.release(), [pool = shared_from_this()](X* px)
		{
			pool->put(px);
		});

		return move(up);
	}

	shared_ptr<X> getShared()
	{
		auto x = _getAvaible();

		shared_ptr<X> sp(x.release(), [pool = shared_from_this()](X* px)
		{
			pool->put(px);
		});

		return sp;
	}

	~Pool()
	{
		auto ul = unique_lock<mutex>(_mtx);
	}


private:

	unique_ptr<X> _getAvaible()
	{
		unique_ptr<X> x;

		for (auto ul = unique_lock<mutex>(_mtx);;)
		{
			if (!_avaible.empty())
			{
				x.swap(_avaible.front());

				_avaible.pop_front();

				_occupied++;

				break;
			}
			else if (_max == 0 || _avaible.size() + _occupied < _max)
			{
				x.swap(_create());

				_occupied++;

				break;
			}
			else
			{
				_cond.wait_for(ul, seconds(1));
			}
		}
			
		return move(x);
	}

	Pool(function<X*()> f, size_t max)
		:_max(max), _create([f](){return move(unique_ptr<X>(f()));})
	{
	}

	Pool(function<unique_ptr<X>()> f, size_t max)
		:_max(max), _create(f)
	{
	}
	

	void put(X* px)
	{
		unique_ptr<X> x(px);

		auto ul = unique_lock<mutex>(_mtx);

		_avaible.push_back(move(x));

		--_occupied;

		ul.unlock();

		_cond.notify_one();
	}

private:
	function<unique_ptr<X>()>		_create;
	size_t							_max = 0;
	mutex							_mtx;
	condition_variable				_cond;
	deque<unique_ptr<X>>			_avaible;
	size_t							_occupied = 0;
};

MYSPACE_END
