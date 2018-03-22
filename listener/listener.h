
#pragma once

#include "myspace/config.h"
#include "myspace/socket/socket.h"

myspace_begin


class listener_t 
{
public:
	
	listener_t(uint16_t port);

	~listener_t();

	shared_ptr<Socket> accept(high_resolution_clock::duration timeout);

	int get_fd();

private:
	int		 _sock = -1;
	uint16_t _port = 0;
};


myspace_end
