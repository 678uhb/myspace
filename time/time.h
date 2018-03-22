
#pragma once

#include "myspace/config.h"

my_space_begin

class Time
{
public:

	string format(time_t t = time(0), const string& fmt = "%F %T");
};

my_space_end
