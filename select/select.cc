
#include "myspace/select/select.h"

my_space_begin


Select::Select()
{
	this->reset();
}


Select& Select::wait(high_resolution_clock::duration duration) 
{
	timeval tv
	{ 
		duration_cast<seconds>(duration).count(),
		duration_cast<microseconds>(duration).count() % 1000000
	};

	::select(_maxfd + 1, &_r, &_w, &_e, &tv);

	this->reset();

	return *this;
}

Select& Select::reset()
{
	FD_ZERO(&_r);
	FD_ZERO(&_w);
	FD_ZERO(&_e);

	_maxfd = -1;

	return *this;
}

my_space_end
