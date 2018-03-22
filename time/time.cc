

#include "myspace/time/time.h"
#include "myspace/memory/memory.h"

my_space_begin

string Time::format(time_t t, const string& fmt)
{
	auto buf = new_unique<char[]>(128);

	struct tm _tm;
#ifdef this_platform_windows
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

my_space_end
