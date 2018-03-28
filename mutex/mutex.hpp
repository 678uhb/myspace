
#pragma once

#include "myspace/myspace_include.h"

myspace_begin

#define if_lock(mtx) if(auto __ul = unique_lock<mutex>(mtx))

#define if_trylock(mtx)									\
	if( auto __ul = [&]() {								\
		auto ul = unique_lock<mutex>(mtx, defer_lock);	\
		ul.try_lock();									\
		return move(ul);}())

myspace_end
