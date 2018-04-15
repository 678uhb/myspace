
#pragma once

#include "myspace/_/stdafx.hpp"

MYSPACE_BEGIN

class Time {
public:
  static std::string format(time_t t, const std::string &fmt = "%F %T");

  template <class Function, class... Arguments>
  static std::chrono::high_resolution_clock::duration
  costs(Function &&func, Arguments &&... args);
};

#define MYSPACE_IF_PAST_SECONDS(x)                                             \
  if ([]() {                                                                   \
        static time_t t = 0;                                                   \
        auto now = time(0);                                                    \
        if (t + (x) > now) {                                                   \
          return false;                                                        \
        } else {                                                               \
          t = now;                                                             \
          return true;                                                         \
        }                                                                      \
      }())



inline std::string Time::format(time_t t, const std::string &fmt) {
  auto buf = new_unique<char[]>(128);

  struct tm _tm;
#if defined(MYSPACE_WINDOWS)
  ::localtime_s(&_tm, &t);
#else
  ::localtime_r(&t, &_tm);
#endif

  std::string ret;

  auto n = ::strftime(buf.get(), 128, fmt.c_str(), &_tm);

  if (n > 0)
    ret.assign(buf.get(), n);

  return ret;
}

template <class Function, class... Arguments>
inline std::chrono::high_resolution_clock::duration
Time::costs(Function &&func, Arguments &&... args) {
  auto begin = std::chrono::high_resolution_clock::now();
  func(std::forward<Arguments>(args)...);
  return std::chrono::high_resolution_clock::now() - begin;
}



MYSPACE_END
