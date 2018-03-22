
#pragma once

#include "myspace/config.h"
#include "myspace/strings/strings.h"

my_space_begin

class Exception : public exception 
{
public:

	template<class... types>
	Exception(const char* file, int line, const string& types...)
	{
		StringStream ss;
		ss << file << ":" << line << " " << types...;
		_desc = move(ss.str());
	}

	static void throwNested(exception&& e)
	{
		if (current_exception())
		{
			throw_with_nested(move(e));
		}
		else
		{
			throw move(e);
		}
	}

private:
	string	_desc;
};





#define throw_syserror(ec) do { \
    throw_e(syserror_t(__FILE__,__LINE__,ec, get_error_msg(ec)));  \
} while(0)
#define throw_syserror_if(x) if((x)) do { \
	auto ec = getlasterror();  \
    throw_e(syserror_t(__FILE__,__LINE__,ec, get_error_msg(ec)));  \
} while(0)



my_space_end
