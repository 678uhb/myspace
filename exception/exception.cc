
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


myspace_end



