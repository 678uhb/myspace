
#pragma once

#include "myspace/_/stdafx.hpp"
#include "myspace/defer/defer.hpp"

MYSPACE_BEGIN

class Error {
public:
  static std::error_code lastError();

  static std::string strerror(const std::error_code &ec);

  static std::string strerror(int e);
};
inline std::error_code Error::lastError() {
#if defined(MYSPACE_WINDOWS)
  return std::error_code(::GetLastError(), std::system_category());
#else
  return std::error_code(errno, std::generic_category());
#endif
}

inline std::string Error::strerror(const std::error_code &ec) {
  return ec.message();
}

inline std::string Error::strerror(int e) {
#if defined(MYSPACE_WINDOWS)
  return Error::strerror(std::error_code(e, std::system_category()));
#else
  return Error::strerror(std::error_code(e, std::generic_category()));
#endif
}

MYSPACE_END
