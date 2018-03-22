
#pragma once

#include "myspace/config.h"

myspace_begin

class Select 
{
public:
	enum detect_type_t 
	{
		detect_read = 1,
		detect_write = 1 << 1,
		detect_exception = 1 << 2,
	};

	Select();

	template<class type>
	Select& push(type x, detect_type_t dt = detect_read) {
		_maxfd = max(_maxfd, x->get_fd());
		if (dt & detect_read) FD_SET(x->get_fd(), &_r);
		if (dt & detect_write) FD_SET(x->get_fd(), &_w);
		if (dt & detect_exception) FD_SET(x->get_fd(), &_e);
		return *this;
	}

	Select& wait(high_resolution_clock::duration duration);

private:
	
	Select& reset();

private:
	int				_maxfd = -1;
	fd_set			_r, _w, _e;
};


myspace_end
