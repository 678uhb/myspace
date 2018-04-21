
#pragma once

#include "myspace/_/stdafx.hpp"
#include "myspace/exception/exception.hpp"
#include "myspace/path/path.hpp"
#include "myspace/process/process.hpp"
#include "myspace/strings/strings.hpp"
MYSPACE_BEGIN

#if defined(MYSPACE_WINDOWS)
typedef struct _stat FileStatus;
#endif

#if defined(MYSPACE_LINUX)
typedef struct stat FileStatus;
#endif

class OS {
public:
  MYSPACE_EXCEPTION_DEFINE(OSError, myspace::Exception)
public:
  static size_t filesize(const std::string &path) noexcept(false);

  // try resurively
  static void makedirs(const std::string &path) noexcept(false);

  // one shot
  static void makedir(const std::string &path) noexcept(false);

  static std::shared_ptr<FileStatus>
  status(const std::string &path) noexcept(false);

  static bool isdir(const std::string path);
  static bool isfile(const std::string path);

private:
  static void makedirs(std::deque<std::string> &left,
                       std::deque<std::string> &right) noexcept(false);
};

inline std::shared_ptr<FileStatus>
OS::status(const std::string &t_path) noexcept(false) {
  auto st = newShared<FileStatus>();
#if defined(MYSPACE_WINDOWS)
  MYSPACE_THROW_IF_EX(OSError, 0 != _stat(t_path.c_str(), st.get()), " ",
                      t_path);
#endif

#if defined(MYSPACE_LINUX)
  MYSPACE_THROW_IF_EX(OSError, 0 != stat(t_path.c_str(), st.get()), " ",
                      t_path);
#endif
  return st;
}

inline bool OS::isdir(const std::string path) {
  try {
    return (OS::status(path)->st_mode & S_IFDIR) == S_IFDIR;
  } catch (...) {
  }
  return false;
}

inline bool OS::isfile(const std::string path) {
  try {
    return (OS::status(path)->st_mode & S_IFREG) == S_IFREG;
  } catch (...) {
  }
  return false;
}

inline void OS::makedir(const std::string &path) noexcept(false) {
#if defined(MYSPACE_LINUX)
  MYSPACE_THROW_IF_EX(OSError, 0 != ::mkdir(path.c_str(), 0777), " ", path);
#endif
#if defined(MYSPACE_WINDOWS)
  MYSPACE_THROW_IF_EX(OSError, 0 != ::_mkdir(path.c_str()), " ", path);
#endif
}
inline void OS::makedirs(std::deque<std::string> &left,
                         std::deque<std::string> &right) noexcept(false) {
  auto path = Strings::deduplicate(Strings::join(left, '/'), '/');
  try {
    OS::makedir(path.c_str());
    if (right.empty())
      return;
    left.emplace_back(move(right.front()));
    right.pop_front();
    OS::makedirs(left, right);
  } catch (...) {
    auto e = Error::lastError();
    if (e == std::errc::no_such_file_or_directory) {
      MYSPACE_THROW_IF_EX(OSError, left.empty());
      right.emplace_front(move(left.back()));
      left.pop_back();
      OS::makedirs(left, right);
      return;
    } else if (e == std::errc::file_exists) {
      if (OS::isdir(path)) {
        if (!right.empty()) {
          left.emplace_back(move(right.front()));
          right.pop_front();
          OS::makedirs(left, right);
        }
        return;
      }
    }
    MYSPACE_THROW_EX(OSError);
  }
}
inline void OS::makedirs(const std::string &t_path) noexcept(false) {
  auto path = Path::join(t_path);
  if (path.empty())
    return;
  bool from_root = false;
  if (path[0] == '/') {
    from_root = true;
  }
  auto tokens_left = Strings::splitOf(path, '/');
  if (from_root) {
    tokens_left.emplace_front(1, '/');
  }
  decltype(tokens_left) tokens_right;
  OS::makedirs(tokens_left, tokens_right);
}

inline size_t OS::filesize(const std::string &path) noexcept(false) {
  return (size_t)OS::status(path)->st_size;
}

MYSPACE_END
