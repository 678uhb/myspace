
#pragma once

#include "myspace/config.h"

my_space_begin


template<class Hold,
	class Mtx = mutex,
	class Cond = condition_variable>
	class Critial : public Hold, public Mtx, public Cond
{
public:

	template<class... Targs>
	Critial(Targs&&... args)
		:Hold(forward<Targs>(args)...)
	{
		
	}
};

my_space_end
