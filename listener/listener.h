
#pragma once

#include "myspace/config.h"
#include "myspace/socket/socket.h"

myspace_begin


class Listener
{
public:
	
	Listener(uint16_t port);

	shared_ptr<Socket> accept(high_resolution_clock::duration timeout);

	~Listener();

	int get_fd();

private:
	int		 _sock = Socket();
};


myspace_end
