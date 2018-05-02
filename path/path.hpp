

#pragma once

#include "myspace/_/stdafx.hpp"
#include "myspace/strings/strings.hpp"
MYSPACE_BEGIN

class Path {
public:
  static std::string basename(const std::string &path);

  static std::string basenameNoext(const std::string &path);

  static std::deque<std::string> splitext(const std::string &path);

  static std::string dirname(const std::string &path);

  template <class... Arguments>
  static std::string join(const Arguments &... args);

  template <class... Arguments> static std::string join(Arguments &&... args);

  template <class Iterable,
            typename std::enable_if<
                std::is_same<typename Iterable::value_type, std::string>::value,
                int>::type = 0>
  static std::string join(const Iterable &arr);

private:
  template <class... Arguments>
  static std::string joinImpl(const std::string &left, const std::string &right,
                              Arguments &&... rest);
  template <class... Arguments>
  static std::string joinImpl(std::string &&left, std::string &&right,
                              Arguments &&... rest);

  static const std::string &joinImpl(const std::string &);

  static std::string joinImpl();

  static std::string joinCheck(const std::string &path);
  static std::string joinCheck(std::string &&path);
};

template <class Iterable,
          typename std::enable_if<
              std::is_same<typename Iterable::value_type, std::string>::value,
              int>::type>
inline std::string Path::join(const Iterable &arr) {
  std::string result = Strings::join(arr, '/');
  if (Strings::startWithin(result, "\\/")) {
    result = '/' +
             Strings::stripOf(
                 Strings::deduplicate(Strings::replace(result, "\\", "/"), '/'),
                 '/');
  } else {
    result = Strings::stripOf(
        Strings::deduplicate(Strings::replace(result, "\\", "/"), '/'), '/');
  }
  return result;
}

template <class... Arguments>
inline std::string Path::join(const Arguments &... args) {
  return joinCheck(joinImpl(args...));
}

template <class... Arguments>
inline std::string Path::join(Arguments &&... args) {
  return joinCheck(joinImpl(std::forward<Arguments>(args)...));
}

template <class... Arguments>
inline std::string Path::joinImpl(const std::string &left,
                                  const std::string &right,
                                  Arguments &&... rest) {
  return joinImpl(left + '/' + right, rest...);
}
template <class... Arguments>
inline std::string Path::joinImpl(std::string &&left, std::string &&right,
                                  Arguments &&... rest) {
  return joinImpl(left + '/' + right, std::forward<Arguments>(rest)...);
}

inline std::string Path::joinCheck(const std::string &path) {
  auto result = Strings::deduplicate(Strings::replace(path, "\\", "/"), '/');
  if (result.size() <= 1)
    return result;
  return Strings::rStripOf(result, '/');
}
inline std::string Path::joinCheck(std::string &&path) {
  path = Strings::deduplicate(Strings::replace(path, "\\", "/"), '/');
  if (path.size() <= 1)
    return path;
  return Strings::rStripOf(path, '/');
}

inline const std::string &Path::joinImpl(const std::string &s) { return s; }

inline std::string Path::joinImpl() { return ""; }

inline std::string Path::basename(const std::string &path) {
  auto pos = path.find_last_of("\\/");

  if (pos != std::string::npos)
    return path.substr(pos + 1);

  return path;
}

inline std::string Path::basenameNoext(const std::string &path) {
  return std::move(splitext(basename(path))[0]);
}

inline std::deque<std::string> Path::splitext(const std::string &path) {
  auto pos = path.find_last_of('.');

  if (pos == path.npos) {
    return { path, "" };
  }

  return { path.substr(0, pos), path.substr(pos + 1) };
}

inline std::string Path::dirname(const std::string &path) {
  auto pos = path.find_last_of("/\\");

  if (pos == path.npos)
    return path;

  return path.substr(0, pos);
}
MYSPACE_END
