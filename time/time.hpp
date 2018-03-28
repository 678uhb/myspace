
#pragma once

#include "myspace/myspace_include.h"

myspace_begin

class Time
{
public:

	static string format(time_t t, const string& fmt = "%F %T")
	{
		auto buf = new_unique<char[]>(128);

		struct tm _tm;
#ifdef myspace_windows
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
};

myspace_end
