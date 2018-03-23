
#pragma once

#include "myspace/config.h"

myspace_begin

class Socket
{
public:
	Socket(int sock);

	Socket(const string& addr, high_resolution_clock::duration timeout);

	Socket(const string& addr, uint16_t port, high_resolution_clock::duration timeout);

	~Socket();

	size_t send(const string& data, high_resolution_clock::duration timeout);

	size_t send(const string& data);

	string recv(high_resolution_clock::duration timeout);

	string recv(size_t len, high_resolution_clock::duration timeout);

	string recv(size_t len);

	string recv_until(const string& delm, high_resolution_clock::duration timeout);

	Socket& setblock();

	Socket& setnonblock();

	bool is_blocked(); 

	operator bool();

	int get_fd();

	bool is_connected();

	void close();

private:


	Socket& set_port(uint16_t port);

	Socket& set_addr(const char* ipport);

	Socket& connect(high_resolution_clock::duration timeout);


	Socket& setblock(bool f);

	int getsockerror();

	bool			_isblocked = true;
	int				_sock = -1;
	uint16_t		_port = 0;
	string			_ip;
};

void set_block(int fd, bool f);

void close_socket(int sock);


myspace_end
