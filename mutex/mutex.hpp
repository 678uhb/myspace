
#pragma once

#include "myspace/config.hpp"

MYSPACE_BEGIN


#define if_lock(mtx) \
  if( auto __ul = unique_lock<mutex>(mtx))


MYSPACE_END
