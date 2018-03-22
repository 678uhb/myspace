
#pragma once

#include "myspace/config.h"

myspace_begin

template<class t, class... a>
unique_ptr<t> new_unique(a&&... args)
{
	return make_unique<t>(forward<a>(args)...);

	/*while (true)
	{
		try
		{
			auto p = new t(forward<a&&>(args)...);
			if (p)
			{
				return unique_ptr<t>(p);
			}
		}
		catch (bad_alloc&)
		{
			this_thread::yield();
		}
	}*/
}


template<class t, class... a>
auto new_shared(a&&... args)
{
	return make_shared<t>(forward<a>(args)...);

	/*while (true)
	{
		try
		{
			auto p = new t(forward<a&&>(args)...);
			if (p)
			{
				return shared_ptr<t>(p);
			}
		}
		catch (bad_alloc&)
		{
			this_thread::yield();
		}
	}*/
}

myspace_end

