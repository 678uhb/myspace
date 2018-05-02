
#pragma once

#include "myspace/_/stdafx.hpp"
#if defined(MYSPACE_LINUX)
#include "myspace/detector/_/epoll.hpp"
#endif
#include "myspace/detector/_/select.hpp"

MYSPACE_BEGIN

#if defined(MYSPACE_LINUX)
typedef Epoll
#else
typedef Select
#endif
Detector;

MYSPACE_END
