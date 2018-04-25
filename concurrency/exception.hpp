#pragma once
#include "myspace/_/stdafx.hpp"
#include "myspace/exception/exception.hpp"
MYSPACE_BEGIN
namespace concurrency {
MYSPACE_EXCEPTION_DEFINE(ConcurrencyError, myspace::Exception)
MYSPACE_EXCEPTION_DEFINE(ContainerDestroyed, ConcurrencyError)
MYSPACE_EXCEPTION_DEFINE(TimeOut, ConcurrencyError)
MYSPACE_EXCEPTION_DEFINE(PredicateMeet, ConcurrencyError)

class Factory;
} // namespace concurrency
MYSPACE_END
