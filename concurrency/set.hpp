#pragma once
#include "myspace/_/stdafx.hpp"
MYSPACE_BEGIN
namespace concurrency {
template <class X> using Set<X> = concurrency::Container<X, std::set<X>>;
} // namespace concurrency
MYSPACE_END