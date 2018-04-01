
#include "myspace/myspace_include.h"

MYSPACE_BEGIN

template<class X, class Creator, class Deleter>
class Pool: public enable_shared_from_this<Pool<X, Creator, Deleter>>
{
	using enable_shared_from_this<Pool>::shared_from_this;

	typedef unique_ptr<X, Deleter> StorageType;

	friend class PoolFactory;
public:

	unique_ptr<X, std::function<void(X*)>> getUnique()
	{
		unique_ptr<X, std::function<void(X*)>>
		up(_getAvaible().release(), [pool = shared_from_this()](X* px)
		{
			pool->put(px);
		});

		return move(up);
	}

	shared_ptr<X> getShared()
	{
		shared_ptr<X> 
		sp(_getAvaible().release(), [pool = shared_from_this()](X* px)
		{
			pool->put(px);
		});

		return move(sp);
	}

private:

	StorageType _getAvaible()
	{
		StorageType x(nullptr, _deleter);

		for (auto ul = unique_lock<mutex>(_mtx);;)
		{
			if (!_avaible.empty())
			{
				x.reset(_avaible.front().release());

				_avaible.pop_front();

				_occupied++;

				break;
			}
			else if (_max == 0 || _avaible.size() + _occupied < _max)
			{
				_occupied++;

				return move(_createItem());
			}
			else
			{
				_cond.wait_for(ul, seconds(1));
			}
		}
			
		return move(x);
	}

	Pool(size_t max, Creator&& creator, Deleter&& deleter)
		:_max(max), _creator(forward<Creator>(creator))
		,_deleter(forward<Deleter>(deleter))
	{
	}

	// Pool(size_t max, const Creator& creator,const Deleter& deleter)
	// 	:_max(max), _creator(creator) ,_deleter(deleter)
	// {
	// }
	

	void put(X* px)
	{
		StorageType x(px, _deleter);

		auto ul = unique_lock<mutex>(_mtx);

		_avaible.push_back(move(x));

		--_occupied;

		ul.unlock();

		_cond.notify_one();
	}

	StorageType _createItem()
	{
		return move(StorageType(_creator(), _deleter));
	}

private:
	
	size_t							_max = 0;
	size_t							_occupied = 0;
	mutex							_mtx;
	condition_variable				_cond;
	deque<StorageType>				_avaible;
	Creator							_creator;
	Deleter							_deleter;
};


class PoolFactory
{
	template<class X>
	class DefaultCreator
	{
	public:
		X* operator()() const
		{
			return new X();
		}
	};
public:
	template<class X>
	static auto create(size_t max = 0)
	-> shared_ptr<Pool<X, DefaultCreator<X>, default_delete<X>>>
	{
		typedef Pool<X, DefaultCreator<X>, default_delete<X>> PoolType;

		return shared_ptr<PoolType>(new PoolType(max, DefaultCreator<X>(), default_delete<X>() ));
	}

	template<class X, class Creator>
	static auto create(size_t max, Creator&& creator)
	-> shared_ptr<Pool<X, Creator, default_delete<X>>>
	{
		typedef Pool<X, Creator, default_delete<X>> PoolType;

		return shared_ptr<PoolType>(new PoolType(max, forward<Creator>(creator), default_delete<X>() ));
	}

	template<class X, class Creator>
	static auto create(size_t max, const Creator& creator)
	-> shared_ptr<Pool<X, Creator, default_delete<X>>>
	{
		typedef Pool<X, Creator, default_delete<X>> PoolType;

		return shared_ptr<PoolType>(new PoolType(max, move(Creator(creator)),default_delete<X>() ));
	}

	template<class X, class Creator, class Deleter>
	static auto create(size_t max, Creator&& creator, Deleter&& deleter)
	-> shared_ptr<Pool<X, Creator, Deleter>>
	{
		typedef Pool<X, Creator, Deleter> PoolType;

		return shared_ptr<PoolType>(new PoolType(max, forward<Creator>(creator), forward<Deleter>(deleter)));
	}
};


MYSPACE_END
