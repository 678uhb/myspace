
#include "myspace/exception/exception.h"

myspace_begin


void Exception::Throw(exception& e)
{
	if (current_exception())
	{
		throw_with_nested(e);
	}
	else
	{
		throw move(e);
	}
}

void Exception::Throw()
{
	Throw(*this);
}

void _dump(string& s, const exception& e)
{
	try
	{
		rethrow_if_nested(e);
	}
	catch (const exception& x)
	{
		_dump(s, x);
	}
	catch (...)
	{

	}

	if (!s.empty())
	{
		s.append(1, '\n');
	}
	s.append(e.what());
}

void _dump(string& s)
{
	try
	{
		auto e = current_exception();

		if (e)
		{
			rethrow_exception(e);
		}
	}
	catch (const exception &e)
	{
		_dump(s, e);
	}
	catch (...)
	{

	}
};

string Exception::dump()
{
	string s;
	
	_dump(s);

	return move(s);
}

const char * Exception::what() const noexcept
{
	return _desc.c_str();
}


myspace_end



