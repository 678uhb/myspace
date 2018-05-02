#pragma once
#include "myspace/_/stdafx.hpp"
#include "myspace/concurrency/container.hpp"
#include "myspace/concurrency/exception.hpp"
MYSPACE_BEGIN

namespace concurrency {
template <class X> using Deque = concurrency::Container<X, std::deque>;
} // namespace concurrency

MYSPACE_END
