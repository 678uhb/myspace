
#pragma once

#include "myspace/myspace_include.h"

MYSPACE_BEGIN

class Time
{
public:

	static string format(time_t t, const string& fmt = "%F %T")
	{
		auto buf = new_unique<char[]>(128);

		struct tm _tm;
#ifdef MYSPACE_WINDOWS
		::localtime_s(&_tm, &t);
#else
		::localtime_r(&t, &_tm);
#endif

		string ret;

		auto n = ::strftime(buf.get(), 128, fmt.c_str(), &_tm);

		if (n > 0)
			ret.assign(buf.get(), n);

		return move(ret);
	}

	template<class Function, class... Arguments>
	static high_resolution_clock::duration costs(Function&& func, Arguments&&... args)
	{
		auto begin = high_resolution_clock::now();
		func(forward<Arguments>(args)...);
		return high_resolution_clock::now() - begin;
	}
};


#define MYSPACE_IF_PAST_SECONDS(x)\
if( []()\
{\
	 static time_t t = 0; \
	 auto now = time(0);\
	 if( t + (x) > now ) { return false; } \
	 else { t = now; return true; } \
} ()  )

MYSPACE_END
