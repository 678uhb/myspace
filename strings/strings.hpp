
#pragma once

#include "myspace/_/stdafx.hpp"

MYSPACE_BEGIN

class Strings {
public:
  static std::string tolower(const std::string &src);

  static std::string toupper(const std::string &src);

  static bool isBlank(char c);

  static std::string rStripOf(const std::string &src,
                              const std::string &delme = "");
  template <
      class ToString,
      typename std::enable_if<
          std::is_constructible<std::string, ToString>::value, int>::type = 1>
  static std::string rStripOf(const std::string &src,
                              const ToString &delme = "");

  static std::string rStripOf(const std::string &src, char delme);

  static std::string lStripOf(const std::string &src,
                              const std::string &delme = "");
  template <
      class ToString,
      typename std::enable_if<
          std::is_constructible<std::string, ToString>::value, int>::type = 1>
  static std::string lStripOf(const std::string &src,
                              const ToString &delme = "");

  static std::string lStripOf(const std::string &src, char delme);

  static std::string stripOf(const std::string &src,
                             const std::string &delme = "");
  template <
      class ToString,
      typename std::enable_if<
          std::is_constructible<std::string, ToString>::value, int>::type = 1>
  static std::string stripOf(const std::string &src,
                             const ToString &delme = "");

  static std::string stripOf(const std::string &src, char delme);

  template <class ReturnType, class SourceType, class DelmeType>
  static ReturnType splitOf(const SourceType &, const DelmeType &);

  template <class ReturnType, class SourceType, class X>
  static ReturnType splitOf(const SourceType &, std::initializer_list<X> init);

  template <class ReturnType = std::deque<std::string>>
  static ReturnType splitOf(const std::string &src, const std::string &delme);

  template <class ReturnType = std::deque<std::string>>
  static ReturnType splitOf(const std::string &src, const char &delme);

  static std::deque<std::string> splitBy(const std::string &src,
                                         const std::string &delimiter);

  template <class Iteraterable>
  static std::string join(const Iteraterable &x, const std::string &join_token);

  template <class Iteraterable>
  static std::string join(const Iteraterable &x, const char);

  static bool startWith(const std::string &str, const std::string &token);

  static bool startWithin(const std::string &str, const std::string &token);

  static bool endWith(const std::string &str, const std::string &token);

  static size_t endWithLess(const std::string &str, const std::string &token);

  static std::string replace(const std::string &src, const std::string &from,
                             const std::string &to);
  static std::string deduplicate(const std::string &src, char c);
};
inline std::string Strings::deduplicate(const std::string &src, char r) {
  std::string result;
  for (auto &c : src) {
    if (c != r) {
      result.append(1, c);
    } else if (result.empty() || result.back() != r) {
      result.append(1, c);
    }
  }
  return result;
}
inline std::string Strings::replace(const std::string &src,
                                    const std::string &from,
                                    const std::string &to) {
  if (from == to || from.empty())
    return src;

  std::string result;

  for (size_t pos = 0, lastpos = 0;;) {
    pos = src.find(from, lastpos);
    if (pos != src.npos) {
      result.append(src.substr(lastpos, pos - lastpos));
      result.append(to);
      lastpos = pos + from.size();
    } else {
      result.append(src.substr(lastpos));
      break;
    }
  }
  return result;
}

inline std::string Strings::tolower(const std::string &src) {
  std::string result;

  result.reserve(src.size());

  for (auto &c : src) {
    result.push_back(std::tolower(c));
  }
  return result;
}

inline std::string Strings::toupper(const std::string &src) {
  std::string result;

  result.reserve(src.size());

  for (auto &c : src) {
    result.push_back(std::toupper(c));
  }
  return result;
}

inline std::string Strings::rStripOf(const std::string &src,
                                     const std::string &delme) {
  std::string result(src);
  result.erase(std::find_if(result.rbegin(), result.rend(),
                            [&delme](char ch) {
                              if (delme.empty()) {
                                return !isBlank(ch);
                              } else {
                                return std::none_of(
                                    delme.begin(), delme.end(),
                                    [&ch](char d) { return d == ch; });
                              }
                            })
                   .base(),
               result.end());
  return result;
}

template <class ToString,
          typename std::enable_if<
              std::is_constructible<std::string, ToString>::value, int>::type>
inline std::string Strings::rStripOf(const std::string &src,
                                     const ToString &delme) {
  return Strings::rStripOf(src, std::string{delme});
}

inline std::string Strings::rStripOf(const std::string &src, char delme) {
  return Strings::rStripOf(src, std::string{delme});
}

inline std::string Strings::lStripOf(const std::string &src,
                                     const std::string &delme) {
  std::string result(src);
  result.erase(result.begin(),
               std::find_if(result.begin(), result.end(), [&delme](char ch) {
                 if (delme.empty()) {
                   return !isBlank(ch);
                 } else {
                   return std::none_of(delme.begin(), delme.end(),
                                       [&ch](char d) { return d == ch; });
                 }
               }));
  return result;
}
template <class ToString,
          typename std::enable_if<
              std::is_constructible<std::string, ToString>::value, int>::type>
inline std::string Strings::lStripOf(const std::string &src,
                                     const ToString &delme) {
  return lStripOf(src, std::string{delme});
}

inline std::string Strings::lStripOf(const std::string &src, char delme) {
  return lStripOf(src, std::string{delme});
}

template <class ToString,
          typename std::enable_if<
              std::is_constructible<std::string, ToString>::value, int>::type>
inline std::string Strings::stripOf(const std::string &src,
                                    const ToString &delme) {
  return stripOf(src, std::string{delme});
}

inline std::string Strings::stripOf(const std::string &src, char delme) {
  return stripOf(src, std::string{delme});
}

inline std::string Strings::stripOf(const std::string &src,
                                    const std::string &delme) {
  return rStripOf(lStripOf(src, delme), delme);
}

template <class ReturnType, class SourceType, class DelmeType>
inline ReturnType Strings::splitOf(const SourceType &src,
                                   const DelmeType &delme) {
  ReturnType ret;
  SourceType one;
  for (auto &s : src) {
    if (std::none_of(
            std::begin(delme), std::end(delme),
            [&s](const typename DelmeType::value_type &d) { return s == d; })) {
      std::fill_n(inserter(one, std::end(one)), 1, s);
    } else if (!one.empty()) {
      std::fill_n(std::inserter(ret, std::end(ret)), 1, one);
      one.clear();
    }
  }
  if (!one.empty()) {
    std::fill_n(std::inserter(ret, std::end(ret)), 1, one);
  }
  return ret;
}

inline bool Strings::isBlank(char c) {
  return std::isblank(c) || std::iscntrl(c);
}

template <class ReturnType, class SourceType, class X>
inline ReturnType Strings::splitOf(const SourceType &src,
                                   std::initializer_list<X> init) {
  return splitOf<ReturnType, SourceType, SourceType>(src, SourceType(init));
}

template <class ReturnType>
inline ReturnType Strings::splitOf(const std::string &src,
                                   const std::string &delme) {
  return splitOf<ReturnType, std::string, std::string>(src, delme);
}

template <class ReturnType>
inline ReturnType Strings::splitOf(const std::string &src, const char &delme) {
  return splitOf<ReturnType, std::string, std::string>(src, std::string{delme});
}

template <class Iteraterable>
inline std::string Strings::join(const Iteraterable &x,
                                 const std::string &join_token) {

  bool first = true;
  std::string result;
  for (auto itr = std::begin(x); itr != std::end(x); itr = std::next(itr)) {
    if (!first) {
      result.append(join_token);
    }
    first = false;
    result.append(*itr);
  }
  return std::move(result);
}

template <class Iteraterable>
inline std::string Strings::join(const Iteraterable &x, char join_token) {
  return Strings::join<Iteraterable>(x, std::string{join_token});
}
inline bool Strings::startWithin(const std::string &str,
                                 const std::string &token) {
  if (token.empty())
    return true;
  if (str.empty())
    return false;
  return token.find(str[0]) != token.npos;
}
inline bool Strings::startWith(const std::string &str,
                               const std::string &token) {
  if (token.empty())
    return true;
  if (str.size() < token.size())
    return false;
  return 0 == str.compare(0, token.size(), token);
}
inline bool Strings::endWith(const std::string &str, const std::string &token) {
  if (token.empty())
    return true;
  if (str.size() < token.size())
    return false;
  return 0 == str.compare(str.size() - token.size(), token.size(), token);
}
inline size_t Strings::endWithLess(const std::string &str,
                                   const std::string &token) {
  if (token.empty())
    return 0;
  size_t i = 0;
  if (str.size() > token.size())
    i = str.size() - token.size();
  for (; i < str.size(); ++i) {
    if (0 == str.compare(i, str.npos, token, 0, str.size() - i)) {
      return token.size() - (str.size() - i);
    }
  }
  return token.size();
}

inline std::deque<std::string> Strings::splitBy(const std::string &src,
                                                const std::string &delimiter) {

  std::deque<std::string> result;
  if (src.empty() || delimiter.empty())
    return result;
  size_t lastpos = 0;
  size_t pos = src.find(delimiter);
  for (; pos != src.npos;
       lastpos = pos + delimiter.size(), pos = src.find(delimiter, lastpos)) {
    result.emplace_back(src.substr(lastpos, pos - lastpos));
  }
  if (lastpos < src.size() - 1) {
    result.emplace_back(src.substr(lastpos));
  }
  return result;
}

MYSPACE_END
