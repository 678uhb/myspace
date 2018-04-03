
#pragma once

#include "myspace/myspace_include.h"
#include "myspace/memory/memory.hpp"
#include "myspace/any/any.hpp"
#include "myspace/socket/socketopt.hpp"

MYSPACE_BEGIN


enum DetectType
{
#ifdef MYSPACE_LINUX
	READ = EPOLLIN,
	WRITE = EPOLLOUT,
	EXCEP = EPOLLERR,
	READ_WRITE = READ | WRITE,
#else
	READ = 1,
	WRITE = (1 << 1),
	EXCEP = (1 << 2),
	READ_WRITE = READ | WRITE,
#endif
};

namespace DetectorImpl
{
	struct Candidate
	{
		Any _x;

		DetectType _ev;
	};
}


#ifdef MYSPACE_LINUX
class Epoll
{
public:

	Epoll()
		:_epoll(epoll_create1(0))
	{

	}

	~Epoll()
	{
		::close(_epoll);

		_epoll = -1;
	}

	template<class X>
	bool add(X&& x, DetectType dt = READ)
	{
		if (!x) return false;

		int fd = x->operator int();

		if (_candidates.find(fd) != _candidates.end())
		{
			return false;
		}

		epoll_event ev;
		ev.events = dt;
		ev.data.fd = fd;

		SocketOpt::setBlock(fd, false);

		if (0 == ::epoll_ctl(_epoll, EPOLL_CTL_ADD, fd, &ev))
		{
			_candidates[fd] = { Any(x), dt };

			return true;
		}
		return false;
	}

	template<class X>
	bool mod(X&& x, DetectType dt)
	{
		if (!x) return false;

		int fd = x->operator int();

		auto itr = _candidates.find(fd);

		if (itr == _candidates.end())
			return false;

		if (itr->second._ev == dt)
			return true;

		epoll_event ev;
		ev.events = dt;
		ev.data.fd = fd;

		SocketOpt::setBlock(fd, false);

		if (0 == ::epoll_ctl(_epoll, EPOLL_CTL_MOD, fd, &ev))
		{
			itr->second = { Any(x), dt };

			return true;
		}
		return false;
	}

	template<class X>
	bool aod(X&& x, DetectType dt = READ)
	{
		return add(x, dt) || mod(x, dt);
	}

	template<class X>
	void del(X&& x)
	{
		if (!x)
			return;

		int fd = x->operator int();

		epoll_event ev;
		::epoll_ctl(_epoll, EPOLL_CTL_DEL, fd, &ev);

		_candidates.erase(fd);
	}

	map<uint32_t, deque<Any>> wait()
	{
		return move(wait_ms(-1));
	}

	map<uint32_t, deque<Any>> wait(high_resolution_clock::duration duration)
	{
		auto ms = duration_cast<milliseconds>(duration).count();

		if (ms < 0)
		{
			ms = 0;
		}
			
		return move(wait_ms(ms));
	}

private:

	map<uint32_t, deque<Any>> wait_ms(int ms)
	{
		map<uint32_t, deque<Any>> result;

		auto events = new_unique<epoll_event[]>(_candidates.size());

		auto n = ::epoll_wait(_epoll, events.get(), _candidates.size(), ms);

		for (auto i = 0; i < n; ++i)
		{
			auto ev = events[i].events;

			auto fd = events[i].data.fd;

			result[ev].push_back(_candidates[fd]._x);
		}

		return move(result);
	}

	int	_epoll = -1;

	unordered_map<int, DetectorImpl::Candidate> _candidates;
};

#endif

class Select
{
public:

	template<class X>
	bool add(X&& x, DetectType dt = READ)
	{	
		if (_candidates.size() >= FD_SETSIZE)
		{
			return false;
		}

		int fd = x->operator int();

		if (_candidates.find(fd) != _candidates.end())
		{
			return false;
		}

		SocketOpt::setBlock(fd, false);
			
		_candidates[fd] = { Any(x), dt };

		return true;
	}

	template<class X>
	bool mod(X&& x, DetectType ev)
	{
		int fd = x->operator int();

		auto itr = _candidates.find(fd);

		if (itr == _candidates.end())
		{
			return false;
		}

		SocketOpt::setBlock(fd, false);
			
		itr->second = DetectorImpl::Candidate{ Any(x), ev };

		return true;
	}

	template<class X>
	bool aod(X&& x, DetectType ev = READ)
	{
		return add(x, ev) || mod(x, ev);
	}

	template<class X>
	void del(X&& x)
	{
		int fd = x->operator int();

		_candidates.erase(fd);
	}

	map<uint32_t, deque<Any>> wait()
	{	
		return move(wait_tv(nullptr));
	}

	map<uint32_t, deque<Any>> wait(high_resolution_clock::duration duration)
	{
		if (duration_cast<microseconds>(duration).count() < 0)
		{
			duration = microseconds(0);
		}	

		timeval tv
		{
			duration_cast<seconds>(duration).count(),

			duration_cast<microseconds>(duration).count() % 1000000
		};

		return move(wait_tv(&tv));
	}

private:
	map<uint32_t, deque<Any>> wait_tv(timeval* ptv)
	{
		FD_ZERO(_r.get());
		FD_ZERO(_w.get());
		FD_ZERO(_e.get());

		int maxfd = 0;

		map<uint32_t, deque<Any>> result;

		for (auto& p : _candidates)
		{
			auto& fd = p.first;

			auto& candidate = p.second;

			maxfd = max(maxfd, fd);

			if (candidate._ev & READ) FD_SET(fd, _r.get());

			if (candidate._ev & WRITE) FD_SET(fd, _w.get());

			if (candidate._ev & EXCEP) FD_SET(fd, _e.get());
		}

		auto n = ::select(maxfd + 1, _r.get(), _w.get(), _e.get(), ptv);

		if (n <= 0)
			return move(result);

		for (auto& p : _candidates)
		{
			if (n <= 0)
				break;

			auto& fd = p.first;

			auto& candidate = p.second;

			if (FD_ISSET(fd, _r.get())) result[READ].push_back(candidate._x), n--;

			if (FD_ISSET(fd, _w.get())) result[WRITE].push_back(candidate._x), n--;

			if (FD_ISSET(fd, _e.get())) result[EXCEP].push_back(candidate._x), n--;
		}

		return move(result);
	}

private:
	unordered_map<int, DetectorImpl::Candidate> _candidates;

	// fd_set could be very large, according to FD_SETSIZE
	unique_ptr<fd_set> _r = new_unique<fd_set>();
	unique_ptr<fd_set> _w = new_unique<fd_set>();
	unique_ptr<fd_set> _e = new_unique<fd_set>();
};

#ifdef MYSPACE_LINUX
typedef Epoll Detector;
#else
typedef Select Detector;
#endif

MYSPACE_END
