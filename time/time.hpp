
#pragma once

#include "myspace/config.hpp"

MYSPACE_BEGIN

class Time
{
public:

	static string format(time_t t, const string& fmt = "%F %T")
	{
		auto buf = new_unique<char[]>(128);

		struct tm _tm;
#ifdef MS_WINDOWS
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

MYSPACE_END
