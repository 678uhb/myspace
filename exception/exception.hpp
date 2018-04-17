
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

  static void Throw(const std::exception &e);

  static std::string dump();

private:
  static void _dump(std::string &s, const std::exception &e);

  static void _dump(std::string &s);
};

template <class... Types>
inline Exception::Exception(const char *file, int line, Types &&... types)
    : std::runtime_error(StringStream(Path::basename(file), ":", line, " ",
                                      std::forward<Types>(types)...)
                             .str()) {}

template <class... Types>
inline void Exception::Throw(const char *file, int line, Types &&... types) {
  auto ec = Error::lastError();
  if (ec.value() == 0) {
    Exception::Throw(Exception(file, line, std::forward<Types>(types)...));
  } else {
    auto desc = StringStream(ec.message(), " ", Path::basename(file), ":", line,
                             " ", std::forward<Types>(types)...)
                    .str();
    Exception::Throw(std::system_error(ec, desc));
  }
}
inline void Exception::Throw(const std::exception &e) {
  if (std::current_exception()) {
    throw_with_nested(e);
  } else {
    throw e;
  }
}
inline std::string Exception::dump() {
  try {
    std::string s;
    _dump(s);
    return s;
  } catch (...) {
    MYSPACE_DEV("exception dump !!");
  }
  return std::string{};
}
// inline const char *Exception::what() const  { return desc_.c_str(); }
inline void Exception::_dump(std::string &s, const std::exception &e) {
  try {
    rethrow_if_nested(e);
  } catch (const std::exception &x) {
    _dump(s, x);
  } catch (...) {
  }

  if (s.empty() || s.back() != '\n') {
    s.append(1, '\n');
  }
  s.append(e.what());
}
inline void Exception::_dump(std::string &s) {
  try {
    auto e = std::current_exception();
    if (e) {
      std::rethrow_exception(e);
    }
  } catch (const std::exception &e) {
    _dump(s, e);
  } catch (...) {
  }
}

MYSPACE_END
