
#pragma once

#include "myspace/config.hpp"
#include "myspace/socket/socket.hpp"

MYSPACE_BEGIN


class Listener
{
public:

	Listener()
	{

	}
	
	Listener(uint16_t port)
	{
		auto sock = (int)::socket(AF_INET, SOCK_STREAM, 0);
		Scope xs([&sock]()
		{
			Socket::close(sock);
		});

		SocketOpt::reuseAddr(sock, true);

		sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.s_addr = INADDR_ANY;

		IF_THROW(0 != ::bind(sock, (sockaddr*)&addr, sizeof(addr)));
		IF_THROW(0 != ::listen(sock, 1024));

		xs.dismiss();
		_sock = sock;
	}

	shared_ptr<Socket> accept(high_resolution_clock::duration timeout)
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

			auto sel = new_shared<Detector>();

			sel->add(this);

			sel->wait(timeout - (this_time - begin_time));
		}
		return nullptr;
	}

	~Listener()
	{
		Socket::close(_sock);
	}

	operator int () const
	{
		return _sock;
	}

private:
	int		 _sock = Socket();
};


MYSPACE_END
