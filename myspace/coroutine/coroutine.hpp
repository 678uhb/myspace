
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

	Fiber(Fiber&& f)
	{
		swap(move(f));
	}

	Fiber(const Fiber&) = delete;

	Fiber& operator = (const Fiber&) = delete;

	void swap(Fiber&& f)
	{
		if(&f != this)
		{
			_func.swap(f._func);

			_calleeIsQuit = f._calleeIsQuit; f._calleeIsQuit = true;

			_callerIsQuit = f._callerIsQuit; f._callerIsQuit = true;

			_caller = f._caller; f._caller = nullptr;

			_callee = f._callee; f._callee = nullptr;

			_callerIsFiber = f._callerIsFiber; f._callerIsFiber = true;
		}
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
		auto whoami = ::GetCurrentFiber();

		if (_caller && _caller != whoami)
		{
			::SwitchToFiber(_caller);
		}
		else if (!_calleeIsQuit && _callee && _callee != whoami)
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

class Ucontext
{
public:
	template<class Function, class... Arguments>
	Ucontext(Function&& func, Arguments&&... args)
		:_stackBuffer(new char[8196])
	{
		MYSPACE_DEV("Ucontext()");

		MYSPACE_IF_THROW(::getcontext(_callee.get()) < 0);

		_callee->uc_stack.ss_sp = _stackBuffer.get();

		_callee->uc_stack.ss_size = 8196;

		_callee->uc_link = _caller.get();

		::makecontext(_callee.get(), reinterpret_cast<void(*)(void)>(&Ucontext::proc), 1, this);

		auto f = bind(forward<Function>(func), forward<Arguments>(args)...);

		_func = [f](Ucontext& x) { f(x); };

		MYSPACE_DEV("Ucontext()!!");
	}

	~Ucontext()
	{
		//_callee.release();

		//_caller.release();

		//_stackBuffer.release();

		MYSPACE_DEV("~Ucontext()");

		_callerIsQuit = true;

		while (!_calleeIsQuit)
		{
			resume();
		}

		MYSPACE_DEV("~Ucontext()!!");

		//MYSPACE_DEV(" ? ", (void*)_callee.get());

		//MYSPACE_DEV(" ? ", (void*)_caller.get());

		//MYSPACE_DEV(" ? ", (void*)_stackBuffer.get());
	}

	Ucontext(Ucontext&& u)
	{
		MYSPACE_DEV("Ucontext(Ucontext&& u)");

		swap(move(u));
	}

	Ucontext(const Ucontext&) = delete;

	Ucontext operator = (const Ucontext&) = delete;

	void swap(Ucontext&& f)
	{
		if(&f != this)
		{
			_func.swap(f._func);

			_calleeIsQuit = f._calleeIsQuit; f._calleeIsQuit = true;

			_callerIsQuit = f._callerIsQuit; f._callerIsQuit = true;

			_caller.swap(f._caller);

			_callee.swap(f._callee);

			_stackBuffer.swap(f._stackBuffer);

			_iAmCaller = f._iAmCaller;
		}
	}


	bool callerIsQuit()
	{
		return _callerIsQuit;
	}

	bool calleeIsQuit()
	{
		return _calleeIsQuit;
	}

	void resume()
	{
		if(_iAmCaller && !_calleeIsQuit)
		{
			_iAmCaller = false;

			swapcontext(_caller.get(), _callee.get());
		}
		else if(!_iAmCaller)
		{
			_iAmCaller = true;

			swapcontext(_callee.get(), _caller.get());
		}
	}

private:

	static void proc(void* arg)
	{
		auto caller = (Ucontext*)arg;

		try
		{
			if(caller && caller->_func)
			{
				(caller->_func)(*caller);
			}
		}
		catch(...)
		{

		}

		caller->_calleeIsQuit = true;

		caller->resume();
	}

	bool	_iAmCaller = true;

	bool	_callerIsQuit = false;

	bool	_calleeIsQuit = false;

	function<void(Ucontext&)> _func;

	unique_ptr<ucontext_t> _caller = unique_ptr<ucontext_t>(new ucontext_t);

	unique_ptr<ucontext_t> _callee = unique_ptr<ucontext_t>(new ucontext_t);

	unique_ptr<char[]>	_stackBuffer;
};

#endif

MYSPACE_END
