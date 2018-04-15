
#pragma once

#include "myspace/_/stdafx.hpp"
#if defined(MYSPACE_WINDOWS)
#include "myspace/coroutine/_/fiber.hpp"
#elif defined(MYSPACE_LINUX)
#include "myspace/coroutine/_/ucontext.hpp"
#endif

MYSPACE_BEGIN

#if defined(MYSPACE_WINDOWS)
typedef Fiber
#elif defined(MYSPACE_LINUX)
typedef Ucontext
#endif
    Coroutine;

MYSPACE_END
