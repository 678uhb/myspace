

#pragma once

#if defined(_WIN32)
#define MYSPACE_WINDOWS
#elif defined(__linux__)
#define MYSPACE_LINUX
#endif

// system headers
#if defined(MYSPACE_WINDOWS)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#undef FD_SETSIZE
#define FD_SETSIZE 1024 * 1024

#include <Objbase.h>
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <Windows.h>
#include <corecrt_io.h>
#include <direct.h>
#pragma comment(lib, "ws2_32.lib")

#elif defined(MYSPACE_LINUX)

#include <arpa/inet.h>
#include <fcntl.h>
#include <iconv.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <ucontext.h>
#include <unistd.h>
#include <uuid/uuid.h>
#else

#error "unknown platform"

#endif

// c headers
#include <cctype>
#include <cerrno>
#include <csetjmp>
#include <cstdarg>
#include <cstdint>
#include <cstring>
// cpp headers
#include <algorithm>
#include <atomic>
#include <bitset>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <exception>
#include <fstream>
#include <functional>
#include <future>
#include <iomanip>
#include <ios>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <system_error>
#include <thread>
#include <tuple>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
using namespace std;
using namespace chrono;
using namespace this_thread;
using namespace placeholders;

// C++ pre-C++98: __cplusplus is 1.
// C++98: __cplusplus is 199711L.
// C++98 + TR1: This reads as C++98 and there is no way to check that I know of.
// C++11: __cplusplus is 201103L.
// C++14: __cplusplus is 201402L.
// C++17: __cplusplus is 201703L.
// #if defined(MYSPACE_WINDOWS) && defined(MYSPACE_SHARED)

// #ifdef MYSPACE_EXPORTS
// #define MYSPACE_API __declspec(dllexport)
// #else
// #define MYSPACE_API __declspec(dllimport)
// #endif

// #else

// #define MYSPACE_API

// #endif

#define MYSPACE_BEGIN namespace myspace {
#define MYSPACE_END }
