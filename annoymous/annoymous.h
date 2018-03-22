
#pragma once

#include "myspace/config.h"

myspace_begin

#define __annoymous_t(type,token,line)	type token##line
#define _annoymous_t(type,line)  __annoymous_t(type,annoymous,line)
#define Annoymous(type)   _annoymous_t(type, __LINE__)

myspace_end