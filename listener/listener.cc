
#include "myspace/listener/listener.h"
#include "myspace/memory/memory.h"
#include "myspace/select/select.h"
#include "myspace/exception/exception.h"
#include "myspace/scope/scope.h"
myspace_begin


Listener::Listener(uint16_t port) 
{
	auto sock =  (int)::socket(AF_INET, SOCK_STREAM, 0);
	Scope xs([&sock]() 
	{
		closeSocket(sock);
	});
	SocketOpt::setBlock(sock, false);
	SocketOpt::reuseAddr(sock, true);
	

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;
	
	if_throw(0 != ::bind(sock, (sockaddr*)&addr, sizeof(addr)));
	if_throw(0 != ::listen(sock, 1024));

	xs.dismiss();
	_sock = sock;
}

Listener::~Listener() 
{
	closeSocket(_sock);
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
