
#pragma once

#include "myspace/config.hpp"

MYSPACE_BEGIN

#define __annoymous_t(type,token,line)	type token##line
#define _annoymous_t(type,line)  __annoymous_t(type,annoymous,line)
#define ANNOYMOUS(type)   _annoymous_t(type, __LINE__)

MYSPACE_END