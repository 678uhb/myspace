
#pragma once

#include "myspace/config.h"
#include "myspace/strings/strings.h"

myspace_begin

class Exception : public exception 
{
public:

	template<class... Types>
	Exception(const char* file, int line, Types&&... types)
	{
		StringStream ss;
		ss << file << ":" << line << " ";
		ss.put(forward<Types>(types)...);
		_desc = move(ss.str());
	}

	template<class... Types>
	static void Throw(const char* file, int line, Types&&... types)
	{
		Exception(file, line, forward<Types>(types)...).Throw();
	}

	static void Throw(exception& e);

	void Throw();

private:
	string	_desc;
};


#define ethrow(...) Exception::Throw(__FILE__, __LINE__, ##__VA_ARGS__);

#define if_throw(x) if((x)) ethrow(#x);


myspace_end
