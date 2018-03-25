
#pragma once

#include "myspace/config.hpp"
#include "myspace/annoymous/annoymous.hpp"

MYSPACE_BEGIN

class Scope 
{
public:
	
	template<class f_t, class... a_t>
	Scope(f_t&& f, a_t&&... args)
		:_f(bind(forward<f_t&&>(f), forward<a_t&&>(args)...)) 
	{
	}

	~Scope()
	{ 
		if (_f)
		{
			try 
			{
				_f(); 
			}
			catch (...)
			{
			};
		}
	}
	
	void dismiss() 
	{ 
		_f = nullptr; 
	}


private:
	function<void()> _f = nullptr;
};


#define DEFER(f) ANNOYMOUS(Scope)([&](){f;})

MYSPACE_END