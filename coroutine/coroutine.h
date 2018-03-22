
#pragma once

#include "myspace/config.h"

my_space_begin

template<class type>
class coroutine_t {
	friend class coroutine_pool_t;
public:
	template<class functype>
	coroutine_t(functype&& fn)
		:_callee_fn(std::forward<functype>(fn)) {
		this->_caller = ::ConvertThreadToFiberEx(this, FIBER_FLAG_FLOAT_SWITCH);
		if (!this->_caller) {
			this->_caller_is_callee = true;
			this->_caller = this_coroutine::get_id();
		}

		this->_callee = CreateFiberEx(0, 0, FIBER_FLAG_FLOAT_SWITCH, &coroutine_t::callee_proc, this);
		::SwitchToFiber(this->_callee);
	}

	~coroutine_t() {
		clean_callee();
	}

	operator bool() {
		return _callee != nullptr;
	}

	type yield(const type& x) {
		_yield_type = x;
		switch_to_caller();
		return move(_resume_type);
	}

	type yield(type&& x) {
		_yield_type = move(x);
		switch_to_caller();
		return move(_resume_type);
	}


	coroutine_t<type>& resume(const type& x) {
		_resume_type = x;
		switch_to_callee();
		return *this;
	}

	coroutine_t<type>& resume(type&& x) {
		_resume_type = move(x);
		switch_to_callee();
		return *this;
	}

	type get() {
		return move(_yield_type);
	}

private:

	void switch_to_callee() {
		if (this->_callee && this->_callee != this_coroutine::get_id()) {
			::SwitchToFiber(this->_callee);
		}
	}

	void switch_to_caller() {
		if (this->_caller && this->_caller != this_coroutine::get_id())
			::SwitchToFiber(this->_caller);
	}

	void clean_callee() {
		switch_to_caller();
		if (!_callee_quit) {
			_callee_quit = true;
			if (_callee) {
				::DeleteFiber(_callee);
				_callee = nullptr;
			}
		}
		if (!this->_caller_is_callee)
			::ConvertFiberToThread();
	}

	static void CALLBACK callee_proc(_In_ PVOID lpParameter) {
		auto co = (coroutine_t*)lpParameter;
		if (this_coroutine::get_id() == co->_callee) {
			co->_callee_fn(*co);
			co->clean_callee();
		}
	}

	type										_resume_type;
	type										_yield_type;
	bool										_caller_is_callee = false;
	bool										_callee_quit = false;
	LPVOID										_callee = nullptr;
	LPVOID										_caller = nullptr;
	function<void(coroutine_t<type>&)>			_callee_fn;
};



//////////////////////////////////////////////////////////////  windows
#ifdef this_platform_windows

#include <mmsystem.h>
#pragma comment(lib, "Winmm.lib")

namespace this_coroutine
{
	LPVOID get_id() 
	{
		return ::GetCurrentFiber();
	}
}

#endif



my_space_end
