
#pragma once

#include "myspace/config.h"

myspace_begin

class Time
{
public:

	static string format(time_t t = time(0), const string& fmt = "%F %T");
};

myspace_end
