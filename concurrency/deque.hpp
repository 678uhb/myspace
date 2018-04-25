#pragma once
#include "myspace/_/stdafx.hpp"
#include "myspace/concurrency/exception.hpp"
MYSPACE_BEGIN

namespace concurrency {
template <class X> using Deque<X> = concurrency::Container<X, std::deque<X>>;
} // namespace concurrency

MYSPACE_END
