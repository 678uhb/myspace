
#pragma once

#include "myspace/config.h"

myspace_begin


class Error
{
public:
	enum 
	{
#ifdef this_platform_windows
		wouldblock = WSAEWOULDBLOCK,
		//err_again = EAGAIN,
		inprogress = WSAEINPROGRESS,
		intr = WSAEINTR,
		already = WSAEALREADY,
		isconn = WSAEISCONN
#else
		wouldblock = EWOULDBLOCK,
		//err_again	= EAGAIN,
		inprogress = EINPROGRESS,
		intr = EINTR,
		already = EALREADY,
		isconn = EISCONN,
#endif
	};


	static int lastNetError();


	string strerror(int ec);

};


myspace_end
