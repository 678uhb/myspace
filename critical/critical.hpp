
#pragma once

#include "myspace/myspace_include.h"

MYSPACE_BEGIN


template<class Hold,
	class Mtx = mutex,
	class Cond = condition_variable>
	class Critical : public Hold, public Mtx, public Cond
{
public:

	template<class... Targs>
	Critical(Targs&&... args)
		:Hold(forward<Targs>(args)...)
	{
		
	}
};

MYSPACE_END
