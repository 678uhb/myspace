
#include "myspace/config.h"

my_space_begin


#define if_lock(mtx) \
  if( auto __ul = unique_lock<mutex>(mtx))


my_space_end
