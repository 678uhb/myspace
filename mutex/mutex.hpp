
#pragma once

#include "myspace/config.hpp"

MYSPACE_BEGIN

#define if_lock(mtx) if(auto __ul = unique_lock<mutex>(mtx))

#define if_trylock(mtx)									\
	if( auto __ul = [&]() {								\
		auto ul = unique_lock<mutex>(mtx, defer_lock);	\
		ul.try_lock();									\
		return move(ul);}())

MYSPACE_END
