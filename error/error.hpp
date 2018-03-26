
#pragma once

#include "myspace/config.hpp"
#include "myspace/scope/scope.hpp"

MYSPACE_BEGIN


class Error
{
public:
	enum 
	{
#ifdef MS_WINDOWS
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

	static int lastNetError()
	{
#ifdef MS_WINDOWS
		int ec = ::WSAGetLastError();
		if (ec) return ec;
		return ::GetLastError();
#else
		return errno;
#endif
	}


	static string strerror(int ec)
	{
#ifdef MS_WINDOWS
		LPVOID buf = nullptr;
		DEFER(::LocalFree(buf));
		::FormatMessage(
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
			nullptr, ec, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&buf, 0, nullptr);
		return move(string((char*)buf));
#else
		auto buf = new_unique<char[]>(1024);
		return move(string(::strerror_r(ec, buf.get(), 1024)));
#endif
	}

};


MYSPACE_END