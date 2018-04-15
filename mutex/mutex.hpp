
#pragma once

#include "myspace/_/stdafx.hpp"

MYSPACE_BEGIN

#define MYSPACE_IF_LOCK(mtx) if (auto __ul = std::unique_lock<std::mutex>(mtx))

#define MYSPACE_IF_TRYLOCK(mtx)                                                \
  if (auto __ul = [&]() {                                                      \
        auto ul = std::unique_lock<std::mutex>(mtx, defer_lock);               \
        ul.try_lock();                                                         \
        return std::move(ul);                                                  \
      }())

#define MYSPACE_FOR_LOCK(mtx)                                                  \
  for (auto __ul = std::unique_lock<std::mutex>(mtx); __ul;)

#define MYSPACE_SYNCHRONIZED                                                   \
  if (auto __ul = []() {                                                       \
        static std::mutex mtx;                                                 \
        return std::unique_lock<std::mutex>(mtx);                              \
      }())

MYSPACE_END
