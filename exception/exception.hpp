
#pragma once

#include "myspace/_/stdafx.hpp"
#include "myspace/error/error.hpp"
#include "myspace/path/path.hpp"
#include "myspace/strings/sstream.hpp"
#include "myspace/strings/strings.hpp"

MYSPACE_BEGIN

#define MYSPACE_EXCEPTION_DEFINE(Child, Base)                                  \
  class Child : public Base {                                                  \
  public:                                                                      \
    template <class... Types>                                                  \
    Child(const char *file, int line, Types &&... types)                       \
        : Base(file, line, std::forward<Types>(types)...) {}                   \
  };

#define MYSPACE_THROW(...)                                                     \
  myspace::Exception::Throw(__FILE__, __LINE__, ##__VA_ARGS__);

#define MYSPACE_THROW_EX(except, ...)                                          \
  myspace::Exception::Throw(except(__FILE__, __LINE__, ##__VA_ARGS__))

#define MYSPACE_THROW_IF(x, ...)                                               \
  if ((x))                                                                     \
    myspace::Exception::Throw(__FILE__, __LINE__, #x, ##__VA_ARGS__);

#define MYSPACE_THROW_IF_EX(except, x, ...)                                    \
  if ((x))                                                                     \
    myspace::Exception::Throw(except(__FILE__, __LINE__, #x, ##__VA_ARGS__));

class Exception : public std::runtime_error {
public:
  template <class... Types>
  Exception(const char *file, int line, Types &&... types);

  template <class... Types>
  static void Throw(const char *file, int line, Types &&... types);

  template <class X> static void Throw(X &&x);

  static std::string dump();

private:
  template <class X> static void _dump(std::string &s, X &&x);
};

template <class... Types>
inline Exception::Exception(const char *file, int line, Types &&... types)
    : std::runtime_error(
          StringStream(Path::basename(file), ":", line,
                       (Error::lastError().value() == 0
                            ? " "
                            : " " + Error::strerror(Error::lastError()) + " "),
                       std::forward<Types>(types)...)
              .str()) {}

template <class... Types>
inline void Exception::Throw(const char *file, int line, Types &&... types) {
  Exception::Throw(Exception(file, line, std::forward<Types>(types)...));
}

template <class X> inline void Exception::Throw(X &&x) {
  if (std::current_exception()) {
    std::throw_with_nested(std::forward<X>(x));
  } else {
    throw x;
  }
}
inline std::string Exception::dump() {
  std::string s;
  try {
    auto eptr = std::current_exception();
    if (eptr) {
      std::rethrow_exception(eptr);
    }
  } catch (const std::exception &e) {
    _dump(s, e);
  } catch (...) {
  }
  return s;
}
template <class X> inline void Exception::_dump(std::string &s, X &&x) {
  try {
    std::rethrow_if_nested(x);
  } catch (const std::exception &e) {
    _dump(s, e);
  } catch (...) {
  }

  if (!s.empty())
    s.append("  ---  ");
  s.append(x.what());
}
MYSPACE_END
