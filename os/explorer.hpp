
#pragma once

#include "myspace/_/stdafx.hpp"
#include "myspace/exception/exception.hpp"
#include "myspace/path/path.hpp"
#include "myspace/process/process.hpp"
#include "myspace/strings/strings.hpp"
MYSPACE_BEGIN
class Explorer {
public:
  Explorer(const std::string &openAt = Process::cwd());
  // current location
  std::string current() const;
  std::string jump(const std::string);
  std::string list();

private:
  std::deque<std::string> last_;
  std::deque<std::string> current_;
};

inline Explorer::Explorer(const std::string &openAt) {
  auto path = Path::join(openAt);
  bool from_root = false;
  if (Strings::startWith(path, "/")) {
    from_root = true;
  }
  current_ = Strings::splitOf(path, '/');
  if (from_root) {
    current_.push_front("/");
  }
}
inline std::string Explorer::current() const { return Path::join(current_); }

MYSPACE_END