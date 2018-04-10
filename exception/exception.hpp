
#pragma once

#include "myspace/_/include.hpp"
#include "myspace/path/path.hpp"
#include "myspace/strings/strings.hpp"

MYSPACE_BEGIN

class Exception {
public:
  // template <class... Types>
  // Exception(const char *file, int line, Types &&... types) noexcept;

  template <class... Types>
  static void Throw(const char *file, int line, Types &&... types);

  static void Throw(const exception &e);

  static string dump();

  // const char *what() const noexcept override;

private:
  static void _dump(string &s, const exception &e);

  static void _dump(string &s);

  // string desc_;
};

// template <class... Types>
// inline Exception::Exception(const char *file, int line,
//                        Types &&... types) noexcept
//  : runtime_error(StringStream(Path::basename(file), ":", line, " ",
//                           forward<Types>(types)...)
//                .str()) {

// StringStream ss;
// ss << Path::basename(file) << ":" << line << " ";
// ss.put(forward<Types>(types)...);
// desc_ = move(ss.str());
//}

template <class... Types>
inline void Exception::Throw(const char *file, int line, Types &&... types) {
  StringStream ss(Path::basename(file), ":", line, " ",
                  forward<Types>(types)...);
  runtime_error e(ss.str().c_str());
  Exception::Throw(e);
}
inline void Exception::Throw(const exception &e) {
  if (current_exception()) {
    throw_with_nested(e);
  } else {
    throw e;
  }
}
inline string Exception::dump() {
  string s;
  _dump(s);
  return s;
}
// inline const char *Exception::what() const noexcept { return desc_.c_str(); }
inline void Exception::_dump(string &s, const exception &e) {
  try {
    rethrow_if_nested(e);
  } catch (const exception &x) {
    _dump(s, x);
  } catch (...) {
  }

  if (s.empty() || s.back() != '\n') {
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

#define MYSPACE_IF_THROW(x, ...)                                               \
  if ((x))                                                                     \
    myspace::Exception::Throw(__FILE__, __LINE__, #x, ##__VA_ARGS__);

MYSPACE_END
