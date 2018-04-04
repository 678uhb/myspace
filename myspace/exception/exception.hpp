
#pragma once

#include "myspace/myspace_include.h"
#include "myspace/path/path.hpp"
#include "myspace/strings/strings.hpp"

MYSPACE_BEGIN

class Exception : public exception {
public:
  template <class... Types>
  Exception(const char *file, int line, Types &&... types);

  template <class... Types>
  static void Throw(const char *file, int line, Types &&... types);

  static void Throw(exception &e);

  static string dump();

  virtual char const *what() const noexcept;

private:
  static void _dump(string &s, const exception &e);

  static void _dump(string &s);

  string desc_;
};

template <class... Types>
inline Exception::Exception(const char *file, int line, Types &&... types) {
  StringStream ss;
  ss << Path::basename(file) << ":" << line << " ";
  ss.put(forward<Types>(types)...);
  desc_ = move(ss.str());
}

template <class... Types>
inline void Exception::Throw(const char *file, int line, Types &&... types) {
  throw Exception(file, line, forward<Types>(types)...);
}
inline void Exception::Throw(exception &e) {
  if (current_exception()) {
    throw_with_nested(e);
  } else {
    throw e;
  }
}
inline string Exception::dump() {
  string s;

  _dump(s);

  return move(s);
}
inline char const *Exception::what() const noexcept { return desc_.c_str(); }
inline void Exception::_dump(string &s, const exception &e) {
  try {
    rethrow_if_nested(e);
  } catch (const exception &x) {
    _dump(s, x);
  } catch (...) {
  }

  if (!s.empty()) {
    s.append(1, '\n');
  }
  s.append(e.what());
}
inline void Exception::_dump(string &s) {
  try {
    auto e = current_exception();

    if (e) {
      rethrow_exception(e);
    }
  } catch (const exception &e) {
    _dump(s, e);
  } catch (...) {
  }
}

#define MYSPACE_THROW(...)                                                     \
  myspace::Exception::Throw(__FILE__, __LINE__, ##__VA_ARGS__);

#define MYSPACE_IF_THROW(x)                                                    \
  if ((x))                                                                     \
    MYSPACE_THROW(#x);

MYSPACE_END
