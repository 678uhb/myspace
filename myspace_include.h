

#pragma once

#include "myspace/myspace_config.h"
// system headers
#if defined(myspace_windows)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define  NOMINMAX
#endif

#undef FD_SETSIZE
#define FD_SETSIZE  1024*1024

#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <direct.h>
#include <corecrt_io.h>
#include <Objbase.h>
#pragma comment(lib, "ws2_32.lib")

#elif defined(myspace_linux)

#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <uuid/uuid.h>
#include <sys/epoll.h>

#else

#error "unknown platform"

#endif

// c headers
#include <cerrno>
#include <cstdint>
#include <cstdarg>
#include <cstring>
#include <cctype>
#include <csetjmp>
//cpp headers
#include <atomic>
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
#include <typeindex>
#include <map>
#include <set>
#include <type_traits>
using namespace std;
using namespace chrono;
using namespace this_thread;


#define myspace_begin namespace myspace {
#define myspace_end   }
