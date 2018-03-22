
#include "myspace/error/error.h"
#include "myspace/scope/scope.h"

myspace_begin

int Error::lastNetError()
{
#ifdef this_platform_windows
	int ec = ::WSAGetLastError();
	if (ec) return ec;
	return ::GetLastError();
#else
	return errno;
#endif
}



string Error::strerror(int ec)
{
#ifdef this_platform_windows
	LPVOID buf = nullptr;
	defer(::LocalFree(buf));
	::FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		nullptr, ec, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&buf, 0, nullptr);
	return move(string((char*)buf));
#else
	auto buf = new_unique_array<char>(1024);
	return move(string(::strerror_r(ec, buf.get(), 1024)));
#endif
}


myspace_end
