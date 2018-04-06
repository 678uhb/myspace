
#pragma once

#include "myspace/_/include.hpp"

MYSPACE_BEGIN

#define ____myspace_annoymous_t(type, token, line) type token##line
#define __myspace_annoymous_t(type, line)                                      \
  ____myspace_annoymous_t(type, annoymous, line)
#define MYSPACE_ANNOYMOUS(type) __myspace_annoymous_t(type, __LINE__)

MYSPACE_END