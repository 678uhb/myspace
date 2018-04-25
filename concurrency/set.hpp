#pragma once
#include "myspace/_/stdafx.hpp"
MYSPACE_BEGIN
namespace concurrency {
template <class X> using Set = concurrency::Container<X, std::set>;
} // namespace concurrency
MYSPACE_END
