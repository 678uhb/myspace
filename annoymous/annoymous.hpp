
#pragma once

#include "myspace/myspace_include.h"

myspace_begin

#define __annoymous_t(type,token,line)	type token##line
#define _annoymous_t(type,line)  __annoymous_t(type,annoymous,line)
#define ANNOYMOUS(type)   _annoymous_t(type, __LINE__)

myspace_end