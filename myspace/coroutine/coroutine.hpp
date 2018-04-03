
#pragma once

#include "myspace/myspace_include.h"
#include "myspace/logger/logger.hpp"
#include "myspace/exception/exception.hpp"

MYSPACE_BEGIN

#ifdef MYSPACE_WINDOWS

class Fiber
{
public:
	template<class Function, class... Arguments>
	Fiber(Function&& func, Arguments&&... args)
	{
		_caller = ::ConvertThreadToFiberEx(this, FIBER_FLAG_FLOAT_SWITCH);

		if (!_caller)
		{
			_callerIsFiber = true;

			_caller = GetCurrentFiber();
		}

		MYSPACE_IF_THROW(!_caller);

		auto f = bind(forward<Function>(func), forward<Arguments>(args)...);

		_func = [f](Fiber& x) { f(x); };

		_callee = ::CreateFiberEx(1024, 1024, FIBER_FLAG_FLOAT_SWITCH, &Fiber::proc, this);

		MYSPACE_IF_THROW(!_callee);
	}

	Fiber(Fiber&&)
	{
	}

	~Fiber()
	{
		_callerIsQuit = true;

		if (_callee)
		{
			while (!_calleeIsQuit)
			{
				resume();
			}

			::DeleteFiber(_callee);
		}

		if (!_callerIsFiber)
		{
			::ConvertFiberToThread();
		}
	}

	Fiber& resume()
	{
		if (_caller && _caller != ::GetCurrentFiber())
		{
			::SwitchToFiber(_caller);
		}
		else if (!_calleeIsQuit && _callee && _callee != ::GetCurrentFiber())
		{
			::SwitchToFiber(_callee);
		}
		return *this;
	}

	bool callerIsQuit()
	{
		return _callerIsQuit;
	}

	bool calleeIsQuit()
	{
		return _calleeIsQuit;
	}

private:

	static void CALLBACK proc(_In_ PVOID lpParameter)
	{
		Fiber* f = (Fiber*)lpParameter;

		try
		{
			if (f->_func)
			{
				(f->_func)(*f);
			}
		}
		catch (...)
		{
			MYSPACE_ERROR(Exception::dump());
		}

		f->_calleeIsQuit = true;

		f->resume();
	}

	function<void(Fiber&)>	_func;

	bool 	_callerIsFiber = false;

	bool	_callerIsQuit = false;

	bool	_calleeIsQuit = false;

	LPVOID _caller = nullptr;

	LPVOID _callee = nullptr;
};

//
//template<class Function, class... Arguments>
//auto makeFiber(Function&& func, Arguments&&... args)
//{
//	auto f = bind(forward<Function>(func), forward<Arguments>(args)...);
//
//	using ReturnType = decltype(f(Fiber<void>)());
//
//	return Fiber<ReturnType>(forward<Function>(func), forward<Arguments>(args)...);
//}

#endif

#ifdef MYSPACE_LINUX

//class Ucontext
//{
//public:
//	Ucontext()
//	{
//		::getcontext(&_ctx);
//		::makecontext(&_ctx);
//	}
//
//private:
//	ucontext_t _ctx = {0};
//};

#endif

MYSPACE_END
