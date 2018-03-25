
#pragma once

#include "myspace/config.hpp"

MYSPACE_BEGIN

class SocketOpt
{
public:
	static void setBlock(int fd, bool f)
	{
#ifdef MS_LINUX
		auto flags = ::fcntl(fd, F_GETFL, 0);
		if (!f)
			::fcntl(fd, F_SETFL, flags | O_NONBLOCK);
		else
			::fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
#endif
#ifdef MS_WINDOWS
		int iMode = (f ? 0 : 1);
		::ioctlsocket(fd, FIONBIO, (u_long FAR*)&iMode);
#endif
	}

	static void reuseAddr(int sock, bool f)
	{
		int on = (f ? 1 : 0);
		setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));
	}

};

MYSPACE_END

