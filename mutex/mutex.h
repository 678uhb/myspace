
#include "myspace/config.h"

myspace_begin


#define if_lock(mtx) \
  if( auto __ul = unique_lock<mutex>(mtx))


myspace_end
