
#include "myspace/listener/listener.h"
#include "myspace/memory/memory.h"
#include "myspace/select/select.h"
myspace_begin


Listener::Listener(uint16_t port) 
{
	_port = port;
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;
	_sock = (int)::socket(AF_INET, SOCK_STREAM, 0);
	set_block(_sock, true);
	::bind(_sock, (sockaddr*)&addr, sizeof(addr));
	::listen(_sock, 1024);
}

Listener::~Listener() 
{
	close_socket(_sock);
}

shared_ptr<Socket> Listener::accept(high_resolution_clock::duration timeout)
{
	for (auto this_time = high_resolution_clock::now(), begin_time = this_time;
		this_time - begin_time <= timeout;
		this_time = high_resolution_clock::now())
	{
		sockaddr_in addr;
		
		socklen_t	addrlen = sizeof(addr);
		
		auto n = (int)::accept(_sock, (sockaddr*)&addr, &addrlen);
		
		if (n >= 0) 
		{
			auto sock = new_shared<Socket>(n);
			return sock;
		}

		auto sel = new_shared<Select>();

		sel->push(this);

		sel->wait(timeout - (this_time - begin_time));
	}
	return nullptr;
}

int Listener::get_fd() 
{
	return _sock;
}

myspace_end
