
#pragma once

#include "myspace/myspace_include.h"

MYSPACE_BEGIN

#define MYSPACE_IF_LOCK(mtx) if (auto __ul = unique_lock<mutex>(mtx))

#define MYSPACE_IF_TRYLOCK(mtx)                                                \
  if (auto __ul = [&]() {                                                      \
        auto ul = unique_lock<mutex>(mtx, defer_lock);                         \
        ul.try_lock();                                                         \
        return move(ul);                                                       \
      }())

#define MYSPACE_FOR_LOCK(mtx) for (auto __ul = unique_lock<mutex>(mtx); __ul;)

#define MYSPACE_SYNCHRONIZED                                                   \
  if (auto __ul = []() {                                                       \
        static mutex mtx;                                                      \
        return unique_lock<mutex>(mtx);                                        \
      }())

MYSPACE_END
