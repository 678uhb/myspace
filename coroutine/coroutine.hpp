
#pragma once

#include "myspace/myspace_include.h"

MYSPACE_BEGIN

#ifdef myspace_windows

template<class Type>
class Coroutine 
{
public:
	template<class Functype, class... Argstype>
	Coroutine(Functype&& fn, Argstype&&... args)
		:_callee_fn(std::forward<functype>(fn)) 
	{
		this->_caller = ::ConvertThreadToFiberEx(this, FIBER_FLAG_FLOAT_SWITCH);

		if (!this->_caller) 
		{
			this->_caller_is_callee = true;

			this->_caller = this_coroutine::get_id();
		}

		this->_callee = CreateFiberEx(0, 0, FIBER_FLAG_FLOAT_SWITCH, &Coroutine::callee_proc, this);

		::SwitchToFiber(this->_callee);
	}

	~Coroutine() 
	{
		clean_callee();
	}

	operator bool() 
	{
		return _callee != nullptr;
	}

	Type yield(const Type& x)
	{
		_yield_type = x;

		switch_to_caller();

		return move(_resume_type);
	}

	Type yield(Type&& x) 
	{
		_yield_type = move(x);

		switch_to_caller();

		return move(_resume_type);
	}

	Type resume(const type& x) 
	{
		_resume_type = x;

		switch_to_callee();

		return *this;
	}

	Type resume(type&& x)
	{
		_resume_type = move(x);

		switch_to_callee();

		return *this;
	}

	type get() 
	{
		return move(_yield_type);
	}

private:

	void switch_to_callee() 
	{
		if (this->_callee && this->_callee != this_coroutine::get_id()) 
		{
			::SwitchToFiber(this->_callee);
		}
	}

	void switch_to_caller() 
	{
		if (this->_caller && this->_caller != this_coroutine::get_id())
		{
			::SwitchToFiber(this->_caller);
		}
	}

	void clean_callee() 
	{
		switch_to_caller();

		if (!_callee_quit) 
		{
			_callee_quit = true;

			if (_callee) 
			{
				::DeleteFiber(_callee);

				_callee = nullptr;
			}
		}

		if (!this->_caller_is_callee)
		{
			::ConvertFiberToThread();
		}
	}

	static void CALLBACK callee_proc(_In_ PVOID lpParameter) 
	{
		auto co = (Coroutine*)lpParameter;

		if (this_coroutine::get_id() == co->_callee) 
		{
			co->_callee_fn(*co);

			co->clean_callee();
		}
	}

	bool										_caller_is_callee = false;

	bool										_callee_quit = false;

	Type										_resume_type;

	Type										_yield_type;

	LPVOID										_callee = nullptr;

	LPVOID										_caller = nullptr;

	function<void(Coroutine<type>&)>			_callee_fn;
};



#include <mmsystem.hpp>
#pragma comment(lib, "Winmm.lib")

namespace this_coroutine
{
	LPVOID get_id() 
	{
		return ::GetCurrentFiber();
	}
}

#endif



MYSPACE_END
