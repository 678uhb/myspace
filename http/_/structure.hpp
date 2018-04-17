

#pragma once

#include "myspace/_/stdafx.hpp"

MYSPACE_BEGIN

namespace http {
enum Method {
  GET,
  POST,
  PUT,
  DELT,
};

typedef std::map<std::string, std::string> Header;
typedef std::string Body;

} // namespace http

MYSPACE_END
