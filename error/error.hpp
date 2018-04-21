
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
  return std::error_code(errno, std::generic_category());
}

inline std::string Error::strerror(const std::error_code &ec) {
  return ec.message();
}

inline std::string Error::strerror(int e) {
  return Error::strerror(std::error_code(e, std::generic_category()));
}

MYSPACE_END
