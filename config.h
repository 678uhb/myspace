#pragma once
// platform
#if defined(_WIN32) || defined(_WIN64)
#define this_platform_windows
#elif defined(__linux__)
#define this_platform_linux
#else
#define this_platform_unknown
#endif
// compiler
#if defined(_GNUC_)
#define this_compiler_gcc
#elif defined(_MSC_VER)
#define this_compiler_msvc _MSC_VER
#else
#define this_complier_unknown
#endif

// system headers
#if defined(this_platform_windows)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define  NOMINMAX
#endif
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <direct.h>
#include <corecrt_io.h>
#include <Objbase.h>
#pragma comment(lib, "ws2_32.lib")
#elif defined(this_platform_linux)
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <uuid/uuid.h>
#else
#error unknown platform
#endif
// c headers
#include <cerrno>
#include <cstdint>
#include <cstdarg>
#include <cstring>
#include <cctype>
#include <csetjmp>
//cpp headers
#include <algorithm>
#include <bitset>
#include <chrono>
#include <condition_variable>
#include <exception>
#include <fstream>
#include <functional>
#include <future>
#include <ios>
#include <iostream>
#include <iomanip>
#include <list>
#include <string>
#include <sstream>
#include <system_error>
#include <thread>
#include <unordered_set>
#include <unordered_map>
#include <utility>
#include <vector>
#include <tuple>
#include <deque>
using namespace std;
using namespace chrono;
using namespace this_thread;


#define myspace_begin namespace myspace {
#define myspace_end   }
